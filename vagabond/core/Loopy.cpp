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

#include "Loopy.h"
#include "Polymer.h"
#include "Entity.h"
#include "Residue.h"
#include "Sequence.h"
#include "BondLength.h"
#include "SequenceComparison.h"
#include "AtomsFromSequence.h"
#include "Knotter.h"
#include "GeometryTable.h"

#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include "BondSequence.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "files/PdbFile.h"

Loopy::Loopy(Polymer *instance) : _instance(instance)
{
	Entity *entity = instance->entity();
	Sequence *refseq = entity->sequence();
	Sequence *instseq = instance->sequence();

	SequenceComparison comp(refseq, instseq);
	std::string match = comp.matchLine();
	
	int count = 0;
	for (size_t i = 0; i < match.length(); i++)
	{
		if (match[i] == ' ')
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
	std::cout << "Rebuild residues " << start << " to "
	<< end << std::endl;

	std::string str;
	
	for (size_t i = start; i < end; i++)
	{
		str += sc->displayString(0, i);
	}

	std::cout << "Loop sequence to rebuild: " << str << std::endl;
	
	// offset needs to match the instance's sequence
	int offset = 0;
	
	Residue *instance_residue_before = sc->residue(2, start - 1);
	if (instance_residue_before)
	{
		offset = instance_residue_before->id().as_num();
	}

	_loops.push_back(Loop{start, end, str, offset});
}

void Loopy::copyPositions()
{
	AtomGroup *contents = _instance->currentAtoms();
	for (Atom *atom : contents->atomVector())
	{
		atom->addOtherPosition("loop", atom->derivedPosition());
	}
}

void Loopy::processLoop(Loop &loop)
{
	_active = {&loop};
	prepareLoop(loop);
	prepareResources();
	
	for (size_t i = 0; i < 100; i++)
	{
		float best = randomiseLoop();
		std::cout << best << std::endl;
		if (best < 0.4f)
		{
			copyPositions();
			writeOutPositions();
		}
	}
	
	writeOutPositions();
}

void Loopy::writeOutPositions(gemmi::Structure &st, int i)
{
	AtomGroup *contents = _instance->currentAtoms();
	std::string model_name = std::to_string(i + 1);

	for (Atom *atom : contents->atomVector())
	{
		const WithPos &pos = atom->otherPositions("loop");
		atom->setDerivedPosition(pos.samples[i]);
	}

	PdbFile::writeAtomsToStructure(contents, st, model_name);
}

void Loopy::writeOutPositions()
{
	AtomGroup *contents = _instance->currentAtoms();
	if (contents->size() == 0)
	{
		return;
	}

	size_t count = (*contents)[0]->otherPositions("loop").samples.size();
	gemmi::Structure st;

	for (size_t i = 0; i < count; i++)
	{
		writeOutPositions(st, i);
	}

	PdbFile::writeStructure(st, "all.pdb");
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
	
	contents->add(grp->atomVector());
	GeometryTable *gt = &GeometryTable::getAllGeometry();

	auto add_bond = [&gt, &contents](int idx)
	{
		Atom *last_c = contents->atomByIdName(ResidueId(idx), "C");
		Atom *first_n = contents->atomByIdName(ResidueId(idx + 1), "N");

		if (last_c && first_n)
		{
			std::cout << last_c->desc() << " to " << first_n->desc() << std::endl;
			float standard = gt->length(last_c->code(), last_c->atomName(),
			                            first_n->atomName(), true);
			new BondLength(contents, last_c, first_n, standard);
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

	add_bond(loop.offset);
	add_bond(loop.offset + (loop.end - loop.start));

	Knotter knotter(contents, gt);
	knotter.setDoLengths(false);
	knotter.knot();
	
	fix_peptide(loop.offset);
	fix_peptide(loop.offset + (loop.end - loop.start));

	std::cout << "Atoms in updated model: " << contents->size() << std::endl;
	std::cout << "With connecting torsions:" << contents->bondTorsionCount() << std::endl;
	
	contents->recalculate();

	delete grp;
}

void Loopy::grabNewParameters()
{
	std::vector<Parameter *> all;
	all = _resources.sequences->torsionBasis()->parameters();
	Loop &curr = *_active.curr;

	for (Parameter *const &p : all)
	{
		if (p->residueId().as_num() >= curr.start && 
		    p->residueId().as_num() <= curr.end &&
		    !p->isConstrained())
		{
			_active.indices.push_back(_active.mask.size());
			_active.mask.push_back(true);
			_active.params.push_back(p);
		}

		_active.mask.push_back(false);
	}

	_active.values.resize(_active.mask.size());
}

void Loopy::prepareResources()
{
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
	std::cout << _active.params.size() << " new parameters to refine." << std::endl;

	Loop &curr = *_active.curr;
	
	auto filter = [curr](Atom *const &atom) -> bool
	{
		return (atom->residueId().as_num() >= curr.start && 
		        atom->residueId().as_num() <= curr.end);
	};

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareTargetsAsInitial(blocks, filter));
	
}

int Loopy::sendJob(const std::vector<float> &vals)
{
	_setter(vals);
	int ticket = submitJob(true, _active.values);

	return ticket;
}

float Loopy::getResult(int *job_id)
{
	Result *r = _resources.calculator->acquireResult();
	if (!r)
	{
		*job_id = -1;
		return FLT_MAX;
	}
	else
	{
		*job_id = r->ticket;
		float res = r->deviation;
		r->transplantPositions();
		r->destroy();
		return res;
	}
}

int Loopy::submitJob(bool show, const std::vector<float> &vals)
{
	int ticket = getNextTicket();

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(ticket);

	Flag::Calc calc;
	calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions | Flag::DoSuperpose);
	Flag::Extract extract = Flag::Extract(Flag::AtomVector | Flag::Deviation);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook);
	sequences->extract(extract, submit_result, final_hook);
	
	_resources.tasks->addTask(first_hook);

	return ticket;
}

void Loopy::randomise(Loopy::Active &active)
{
	int param_idx = 0;
	for (Parameter *const &p : active.params)
	{
		int idx = active.indices[param_idx];
		
		if (p->isPeptideBond())
		{
			p->setValue(180.f);
			active.values[idx] = 0.f;
		}
		else
		{
			active.values[idx] = (rand() / (double)RAND_MAX) * 360.f - 180.f;
		}

		param_idx++;
	}
}

size_t Loopy::parameterCount()
{
	std::vector<float> tmp;
	_getter(tmp);
	return tmp.size();
}

float Loopy::randomiseLoop()
{
	randomise(_active);
	submitJob(true, _active.values);
	retrieve();
	
	int count = 0;
	float best = FLT_MAX;
	
	while (count < 1)
	{
		setGetterSetters(_active.params, _getter, _setter, true);

		SimplexEngine engine(this);
		std::vector<float> steps = getSteppers(_active.params);
		engine.chooseStepSizes(steps);
		engine.setMaxRuns(1000);
		engine.setMaxJobsPerVertex(1);
		engine.start();

		bool improved = engine.improved();
		if (!improved)
		{
			return FLT_MAX;
		}

		const std::vector<float> &trial = engine.bestResult();
		best = engine.bestScore();
		
		count++;
	}
	
	return best;
}

void Loopy::operator()(int i)
{
	if (i >= _loops.size())
	{
		throw std::runtime_error("Not a loop index for model completion");
	}

	processLoop(_loops[i]);
}

std::vector<float> Loopy::getSteppers(const std::vector<Parameter *> &params)
{
	std::vector<float> steps;
	for (Parameter *const param : params)
	{
		if (!param->coversMainChain())
		{
			continue;
		}
		else if (param->isPeptideBond())
		{
			steps.push_back(2.f);
		}
		else
		{
			steps.push_back(60.f);
		}
	}

	return steps;
}

void Loopy::setGetterSetters(const std::vector<Parameter *> &params,
                             Getter &getter, Setter &setter, bool main_chain)
{
	getter = [&params, main_chain, this](std::vector<float> &values)
	{
		int n = 0;
		for (Parameter *const param : params)
		{
			if (!main_chain || 
			    (param->coversMainChain()))
			{
				int idx = _active.indices[n];
				values.push_back(_active.values[idx]);
				n++;
			}
		}
	};
	
	std::vector<float> start;
	getter(start);

	setter = [&params, start, main_chain, this](const std::vector<float> &values)
	{
		int n = 0;
		for (Parameter *const param : params)
		{
			if (!main_chain || (param->coversMainChain() ))
			{
				int idx = _active.indices[n];
				_active.values[idx] = values[n] + start[n];
				n++;
			}
		}
	};
}
