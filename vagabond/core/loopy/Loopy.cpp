// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include <functional>
#include <gemmi/mmread.hpp>
#include <gemmi/to_pdb.hpp>

#include "Entity.h"
#include "Polymer.h"
#include "Residue.h"
#include "Sequence.h"
#include "BondLength.h"
#include "SequenceComparison.h"
#include "AtomsFromSequence.h"
#include "Knotter.h"
#include "GeometryTable.h"
#include "Parameter.h"
#include "Metadata.h"
#include "Model.h"

#include "engine/MapTransferHandler.h"
#include "engine/ImplicitBHandler.h"
#include "engine/MapSumHandler.h"
#include "engine/CorrelationHandler.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "grids/Diffraction.h"
#include "grids/ArbitraryMap.h"
#include "grids/AtomMap.h"

#include "files/PdbFile.h"
#include "files/MtzFile.h"

#include "RoughLoop.h"
#include "LoopCorrelation.h"
#include "FilterSelfClash.h"
#include "FilterCrystalContact.h"

#include "Loopy.h"

Loopy::Loopy(Polymer *instance) : _instance(instance)
{
	_instance->load();
	findLoops();
	
	Metadata::KeyValues list = instance->metadata();
	if (list.count("rmsd"))
	{
		_rmsdLimit = list["rmsd"].number();
	}

	std::cout << "Using RMSD limit: " << _rmsdLimit << std::endl;
}

Loopy::~Loopy()
{
	_instance->unload();
}

void Loopy::findLoops()
{
	Entity *entity = _instance->entity();
	Sequence *refseq = entity->sequence();
	Sequence *instseq = _instance->sequence();
	
	SequenceComparison comp(refseq, instseq);
	std::string match = comp.matchLine();

	int count = 0;
	for (size_t i = 0; i < match.length(); i++)
	{
		bool increment = false;
		if (match[i] == ' ')
		{
			Residue *instance_residue = comp.residue(2, i);
			if (instance_residue->code() == " ")
			{
				increment = true;
			}
		}
		if (increment)
		{
			count++;
		}
		else
		{
			if (count > 0)
			{
				getLoop(&comp, i - count, i);
			}

			count = 0;
		}
	}
}

void Loopy::getLoop(SequenceComparison *sc, int start, int end)
{
	std::string str;
	
	for (size_t i = start; i < end; i++)
	{
		str += sc->displayString(0, i);
	}

	// offset needs to match the instance's sequence
	int offset = 0;
	
	Residue *instance_residue_before = sc->residue(2, start - 1);
	if (instance_residue_before)
	{
		offset = instance_residue_before->id().as_num();
	}
	
	_loops.push_back(Loop{start, end, str, offset, _instance});

	std::cout << "Rebuild residues " << start << " to "
	<< end << " (reference)" << std::endl;
	std::cout << "Rebuild residues " << _loops.back().instance_start() << " to "
	<< _loops.back().instance_end() << " (instance)" << std::endl;

	std::cout << "Loop sequence to rebuild: " << str << std::endl;
}

void Loopy::copyPositions(const std::string &tag)
{
	AtomGroup *contents = _instance->currentAtoms();
	for (Atom *atom : contents->atomVector())
	{
		atom->addOtherPosition(tag, atom->derivedPosition());
	}
}

ListConformers Loopy::generateRoughs(int n)
{
	RoughLoop rougher(this, _active);

	ListConformers roughs;
	
	while (roughs.size() < n)
	{
		rougher.calculateWeights();

		float best = rougher.roughLoop();
		std::cout << best;

		if (best < _rmsdLimit)
		{
			roughs.push_back(new Conformer(_active));
			roughs.back()->setCount(_count);
			_count++;
			std::cout << " (" << roughs.size() << ")";

			copyPositions("rough");
		}

		std::cout << std::endl;
	}

	writeOutPositions("rough");
	writeOutPositions("no_self_clash");
	writeOutPositions("no_contacts");
	
	std::cout << "Reached " << roughs.size() << " rough solutions." << std::endl;
	return roughs;
}

ListConformers filterConformers(const ListConformers &list,
                                const FilterConformer &filter,
                                float threshold)
{
	ListConformers filtered;

	for (Conformer *conformer : list)
	{
		bool keep = filter(conformer, threshold);
		
		if (keep)
		{
			filtered.push_back(conformer);
		}
	}
	std::cout << std::endl;
	
	return filtered;
}

void sumDensity(ListConformers &list, Loopy *loop, ArbitraryMap *map)
{
	LoopCorrelation correl(map, loop);

	list.do_on_each([&correl](Conformer *conf)
   	{
		float score = correl.scoreFor(conf);
		conf->setCorrelationWithDensity(score);
	});

}

void Loopy::processLoop(Loop &loop)
{
	_active = Conformer(&loop);
	prepareLoop(loop);
	prepareResources();

	while (_noncontact.size() < 1000)
	{
		ListConformers list = generateRoughs(10);

		sumDensity(list, this, _map);
		_generated += list;
		
		ListConformers next;
		next = filterConformers(list, FilterSelfClash(this), 2.3);
		_nonclash += next;

		ListConformers nexter;
		nexter = filterConformers(next, FilterCrystalContact(this), 0.2);
		_noncontact += nexter;
		
		sendResponse("non_clash", &_nonclash);
		sendResponse("non_contact", &_noncontact);

		progressReport();
	}
}

void Loopy::progressReport()
{
	std::cout << "Total generated: " << _generated.size() << std::endl;
	std::cout << "Loops which do not clash internally: " << _nonclash.size() << std::endl;
	std::cout << "Loops which do not clash with rest of crystal contents: " 
	<< _noncontact.size() << std::endl;

}

void Loopy::writeOutPositions(gemmi::Structure &st, const std::string &tag, int i)
{
	AtomGroup *contents = _instance->currentAtoms();
	std::string model_name = std::to_string(i + 1);

	for (Atom *atom : contents->atomVector())
	{
		const WithPos &pos = atom->otherPositions(tag);
		atom->setDerivedPosition(pos.samples[i]);
	}

	PdbFile::writeAtomsToStructure(contents, st, model_name);
}

void Loopy::writeOutPositions(const std::string &tag)
{
	AtomGroup *contents = _instance->currentAtoms();
	if (contents->size() == 0)
	{
		return;
	}

	size_t count = (*contents)[0]->otherPositions(tag).samples.size();
	gemmi::Structure st;

	for (size_t i = 0; i < count; i++)
	{
		writeOutPositions(st, tag, i);
	}

	PdbFile::writeStructure(st, tag + ".pdb");
}

void Loopy::prepareLoop(const Loop &loop)
{
	Sequence fragment(loop.seq, loop.offset);

	AtomsFromSequence afs(fragment, false);
	AtomGroup *grp = afs.atoms();
	grp->setOwns(false);

	std::cout << "Atoms in sequence fragment: " << grp->size() << std::endl;
	std::cout << "Torsions in sequence fragment: " << grp->bondTorsionCount() << std::endl;
	
	AtomGroup *contents = _instance->currentAtoms();

	std::cout << "Atoms in original model: " << contents->size() << std::endl;
	std::cout << "Torsions in original model: " << contents->bondTorsionCount() << std::endl;

	Atom *last_c = contents->atomByIdName(ResidueId(loop.instance_start()), "C");
	std::string chain = last_c->chain();
	grp->do_op([chain](Atom *atom) { atom->setChain(chain); });
	
	contents->add(grp->atomVector());
	GeometryTable *gt = &GeometryTable::getAllGeometry();

	auto add_bond = [&gt, &contents](int idx)
	{
		Atom *last_c = contents->atomByIdName(ResidueId(idx), "C");
		Atom *first_n = contents->atomByIdName(ResidueId(idx + 1), "N");

		if (last_c && first_n)
		{
			std::cout << "Hooking up " << last_c->desc() << " to " << first_n->desc() << std::endl;
			auto standard = gt->length(last_c->code(), last_c->atomName(),
			                            first_n->atomName(), true);
			new BondLength(contents, last_c, first_n, 
			               standard.first, standard.second);
		}
	};

	auto fix_peptide = [&contents](int idx)
	{
		Atom *atom = contents->atomByIdName(ResidueId(idx + 1), "N");

		for (size_t i = 0; i < atom->bondTorsionCount(); i++)
		{
			if (atom->bondTorsion(i)->isPeptideBond())
			{
				atom->bondTorsion(i)->setRefinedAngle(180.f);
			}
		}
	};

	contents->orderByResidueId();

	add_bond(loop.instance_start());
	add_bond(loop.instance_end());

	Knotter knotter(contents, gt);
	knotter.setDoLengths(false);
	knotter.knot();
	
	fix_peptide(loop.instance_start());
	fix_peptide(loop.instance_end());

	std::cout << "Atoms in updated model: " << contents->size() << std::endl;
	std::cout << "With connecting torsions: " << contents->bondTorsionCount() << std::endl;
	std::cout << std::endl;
	
	contents->recalculate();

	delete grp;
}

void Loopy::grabNewParameters()
{
	if (!_active)
	{
		return;
	}

	std::vector<Parameter *> all;
	all = _resources.sequences->torsionBasis()->parameters();
	int i = 0;

	for (Parameter *const &p : all)
	{
		_active.addParameter(p, i);
		i++;
	}
}

void Loopy::loadDensityMap()
{
	Diffraction *diff = _instance->model()->getDiffraction();
	_data = diff;
	
	if (_data)
	{
		_map = new ArbitraryMap(*diff);
		_map->setupFromDiffraction();
	}
}

void Loopy::fCalcMap()
{
	if (!_map)
	{
		return;
	}

	_instance->model()->load();
	AtomGroup *contents = _instance->model()->currentAtoms();
	contents->recalculate();

	BondCalculator *calculator = _resources.calculator;
	
	/* a nothing-hook that does nothing, to get us started */
	Task<void *, void *> *first_hook = new Task<void *, void *>
	([](void *) -> void *
	{
		return nullptr;
	}, "nothing");

	/* prepare end result */
	Task<Result, void *> *submit_result = calculator->actOfSubmission(_ticket);

	/* get list of elements from our allocated resources */
	std::map<std::string, int> list = _resources.sequences->elementList();
	std::map<std::string, std::vector<glm::vec3> *> positions;

	/* for each element, get the associated elements out of the contents */
	for (auto it = list.begin(); it != list.end(); it++)
	{
		const std::string &ele = it->first;
		std::vector<glm::vec3> *vec = new std::vector<glm::vec3>();
		positions[ele] = vec;
	}

	Loop &curr = *_active.loop();
	auto filter = [&curr](Atom *const &atom) -> bool
	{
		return curr.idNeedsClashCheck(atom->residueId());
	};
	
	for (Atom *atom : contents->atomVector())
	{
		std::string e = atom->elementSymbol();
		if (!filter(atom) && positions.count(e)) // may not include e.g. ions
		{
			positions[e]->push_back(atom->initialPosition());
		}
	}

	/* get the element tasks set-up to continue down the normal calculation */

	std::map<std::string, GetEle> eleTasks;
	for (auto it = list.begin(); it != list.end(); it++)
	{
		const std::string &ele = it->first;
		std::vector<glm::vec3> *vecPtr = positions[ele];
		auto grab_elements = [vecPtr](BondSequence *) -> std::vector<glm::vec3> *
		{
			return vecPtr;
		};

		auto *get = new Task<BondSequence *, std::vector<glm::vec3>*>
		(grab_elements, "grab positions for " + ele);

		first_hook->must_complete_before(get);
		eleTasks[ele].get_pos = get;
	}

	/* resume the 'normal' flow for calculating electron density maps */
	MapTransferHandler *const &eleMaps = _resources.perElements;
	eleMaps->extract(eleTasks);

	/* summation of per-element maps into final real-space map */
	MapSumHandler *const &sums = _resources.summations;
	Task<SegmentAddition, AtomMap *> *make_map = nullptr;
	sums->grab_map(eleTasks, submit_result, &make_map);

	_resources.tasks->addTask(first_hook);

	Result *r = calculator->acquireObject();
	AtomMap *map = r->map;

	ArbitraryMap *partial = (*map)();
	// add this to a real space map
	
	_fcMap = new ArbitraryMap(*_map);
	_fcMap->clear();
	
	_fcMap->addFromOther(*partial);
	_fcMap->applySymmetry(_data->spaceGroupName());
	
	MtzFile file;
	file.setMap(_fcMap);
	file.write_to_file("test.mtz");
	
	_instance->model()->unload();
}

void Loopy::prepareMaps(bool all_atoms)
{
	BondSequenceHandler *&sequences = _resources.sequences;

	/* calculate transfer to per-element maps */
	MapTransferHandler *perEles = new MapTransferHandler(sequences->elementList(), 
	                                                     _threads);
	perEles->setCubeDim(0.8);
	
	if (_resources.perElements)
	{
		delete _resources.perElements;
	}

	AtomGroup *contents = _instance->model()->currentAtoms();
	
	if (!all_atoms)
	{
		Loop &curr = *_active.loop();
		auto filter = [&curr](Atom *const &atom) -> bool
		{
			return curr.idInLoop(atom->residueId());
		};

		contents = contents->new_subset(filter);
	}

	perEles->supplyAtomGroup(contents->atomVector());
	perEles->setup();
	_resources.perElements = perEles;
	
	if (!all_atoms)
	{
		delete contents; contents = nullptr;
	}
	
	if (_resources.summations)
	{
		delete _resources.summations;
	}
	
	/* summation of all element maps into one */
	MapSumHandler *sums = new MapSumHandler(_threads, perEles->segment(0));
	sums->setup();
	_resources.summations = sums;
	
	std::vector<float> aniso = {1.f, 1.f, 1.f, 0., 0., 0.};
	/* application of additional anisotropic B factor */
	ImplicitBHandler *ibh = new ImplicitBHandler(_threads, 
	                                             sums->templateMap(), aniso);
	ibh->setup();
	_resources.implicits = ibh;

	if (!all_atoms)
	{
		/* correlation of summed density map against reference */
		CorrelationHandler *cc = new CorrelationHandler(_map, sums->templateMap(),
		                                                _threads);

		cc->setup();
		_resources.correlations = cc;

	}
}

void Loopy::prepareResources()
{
	cleanup();

	_resources.allocateMinimum(_threads);

	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
	}

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();
	
	grabNewParameters();
	std::cout << _active.validCount() << " new parameters to refine." << std::endl;

	prepareMaps(true);
	
	loadDensityMap();
	fCalcMap();

	prepareMaps(false);

	Loop &curr = *_active.loop();
	auto filter = [&curr](Atom *const &atom) -> bool
	{
		return !curr.rightOfLoop(atom->residueId());
	};
	
	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareTargetsAsInitial(blocks, filter));
}


void Loopy::operator()(int i)
{
	if (i >= _loops.size())
	{
		throw std::runtime_error("Not a loop index for model completion");
	}

	processLoop(_loops[i]);
}


