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

#include <algorithm>
#include <stdlib.h>

#include "paths/Route.h"
#include "SubmissionHelp.h"
#include "Polymer.h"
#include "Grapher.h"
#include "ParamSet.h"
#include "Separation.h"
#include "TorsionData.h"
#include "TorsionBasis.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "PairwiseDeviations.h"
#include "EnergyTorsions.h"
#include "GradientTerm.h"
#include "paths/BundleBonds.h"
#include "function_typedefs.h"

#include "matrix_functions.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "BondSequenceHandler.h"

Route::Route(Instance *from, Instance *to, const RTAngles &list)
{
	srand(time(NULL));
	setInstance(from);
	_endInstance = to;
	_source = list;
	
	addLinkedInstances(from, to);
}

Route::Route(const RTAngles &list)
{
	srand(time(NULL));
	_source = list;
}

void Route::addLinkedInstances(Instance *from, Instance *to)
{
	_pairs.push_back({from, to});
	if (_instance == nullptr)
	{
		_instance = from;
		_endInstance = to;
	}
	from->load();
}

AtomGroup *Route::all_atoms()
{
	AtomGroup *grp = new AtomGroup();
	
	for (InstancePair &pair : _pairs)
	{
		AtomGroup *next = pair.start->currentAtoms();
		next->recalculate();
		grp->add(next);
	}

	return grp;
}

Route::~Route()
{
	deleteHelpers();
	
}

void Route::setup()
{
	if (_setup)
	{
		return;
	}

	if (_source.size() == 0 && motionCount() == 0)
	{
		throw std::runtime_error("No destination or prior set for route");
	}

	prepareResources();
	_setup = true;
}

float Route::submitJobAndRetrieve(float frac, bool show)
{
	clearTickets();

	submitToShow(frac);

	retrieve();
	
	float ret = _point2Score.begin()->second.deviations;

	return ret;
}

GradientPath *Route::submitGradients(const CalcOptions &options, int order,
                                     const ValidateIndex &validate,
                                     BondSequenceHandler *handler)
{
	if (handler == nullptr) handler = _hydrogenFreeSequences;
	
	int steps = (order + 1) * 2;

	_gradientBin.holdHorses();

	PairwiseDeviations *pw = _helpers[handler].pairwise.acquire();
	Separation *sep = _helpers[handler].separation.acquire();

	// zero = all calculations
	Task<GradientPath, void *> *big_submission = _gradientBin.actOfSubmission(0);
	Flag::Calc calc = Flag::Calc(Flag::DoTorsions);

	std::vector<AtomBlock> &blocks = handler->sequence()->blocks();
	TorsionBasis *basis = handler->torsionBasis();

	bool clash = (options & VdWClashes);
	if (clash)
	{
		throw std::runtime_error("Not supporting gradients for clash right now");
	}

	std::vector<std::pair<int, int>> idxs;
	idxs = _selection.activeParameters(handler, validate);

	std::vector<int> motion_idxs(idxs.size());
	
	for (int i = 0; i < idxs.size(); i++)
	{
		motion_idxs[i] = idxs[i].second; // gradient position index
	}

	big_submission->input.grads.resize(idxs.size());
	big_submission->input.motion_idxs = motion_idxs;
	big_submission->at_least = (steps - 1) * idxs.size();

	for (size_t i = 0; i < steps; i++)
	{
		if (i == 0) continue;

		float frac = i / (float)steps;

		BaseTask *first_hook = nullptr;
		CalcTask *final_hook = nullptr;
		handler->calculate(calc, {frac}, &first_hook, &final_hook);
		Task<BondSequence *, void *> *let_go = handler->letGo();
		
		for (int j = 0; j < idxs.size(); j++)
		{
			int b_idx = idxs[j].first;
			Parameter *p = basis->parameter(blocks[b_idx].torsion_idx);
			int g_idx = j;
			auto momentum_term = [sep, order, frac, g_idx, b_idx, p, pw]
			(BondSequence *seq) -> GradientTerm
			{
				std::function<Floats(const float &)> weights =
				WayPoints::weights(order);

				GradientTerm term(order, frac, g_idx, b_idx, p, 
				                  weights);
				term.momentum(seq, pw, sep);
				return term;
			};

			auto make_term = 
			(new Task<BondSequence *, GradientTerm> (momentum_term, 
                                         "momentum gradient"));

			final_hook->follow_with(make_term);
			final_hook->follow_with(let_go);
			make_term->follow_with(big_submission);
			make_term->must_complete_before(let_go);
		}

		_resources.tasks->addTask(first_hook);
	}

	_gradientBin.releaseHorses();
	GradientPath *r = _gradientBin.acquireObject();
	
	return r;
}


void Route::prepareAlignment()
{
	if (_noncovs)
	{
		_noncovs->provideSequence(_resources.sequences->sequence());
	}
}

std::vector<std::function<BondSequence *(BondSequence *)>> 
Route::extraTasks(const float &frac)
{
	std::vector<std::function<BondSequence *(BondSequence *)>> ret;
	if (_noncovs && _noncovs->ready())
	{
		auto alignment = _noncovs->align_task(frac);
		ret.push_back(alignment);
	}
	return ret;
}

void Route::applyPostCalcTasks(CalcTask *&hook, const float &frac)
{
	auto tasks = extraTasks(frac);
	for (auto &task : tasks)
	{
		CalcTask *job = new CalcTask(task, "post-calc task");
		hook->follow_with(job);
		hook = job;
	}
}

void Route::submitValue(const CalcOptions &options, int steps)
{
	SubmissionHelp help(this, options);
	help.prepareBinForScoreResult();
	help.torsionPositionCalculation();
	help.setParticularResidues(_ids);
	help.pairwiseWorkIfApplicable();
	help.finaliseJobSubmission();
}

void Route::submitToShow(float frac, Atom *atom)
{
	if (frac < 0)
	{
		frac = _chosenFrac;
	}

	SubmissionHelp help(this, {});
	help.setAcquirePositions(true, frac);
	help.setFocusAtom(atom);
	help.prepareBinForScoreResult();
	help.torsionPositionCalculation();
	help.finaliseJobSubmission();
}

void Route::colourHiddenHinges(float frac)
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		Parameter *const &p = parameter(i);
		if (!p->coversMainChain())
		{
			continue;
		}

		float hinge = motion(i).hiddenHingeAngle(frac);
		
		std::vector<Atom *> reporters;
		for (int j = 0; j < p->atomCount(); j++)
		{
			if (p->atom(j)->isReporterAtom())
			{
				reporters.push_back(p->atom(j));
			}
		}
		
		if (reporters.size() == 0)
		{
			continue;
		}

		hinge /= (reporters.size());
		// so peptide bond will contribute to neighbouring CAs!

		for (Atom *atom : reporters)
		{
			atom->addToColour(hinge);
		}
	}
}

const Grapher &Route::grapher() const
{
	const Grapher &g = _resources.sequences->grapher();
	return g;
}

AtomGraph *Route::grapherForTorsionIndex(int idx)
{
	AtomGraph *ag = grapher().graph(parameter(idx));

	return ag;
}

void Route::setFlips(std::vector<int> &idxs, std::vector<int> &fs)
{
	for (size_t j = 0; j < idxs.size(); j++)
	{
		setFlip(idxs[j], fs[j]);
	}
}

void Route::getParametersFromBasis(const MakeMotion &make_mot)
{
	ParamSet missing;

	std::vector<Motion> tmp_motions;
	std::vector<ResidueTorsion> torsions;

	TorsionBasis *basis = _resources.sequences->torsionBasis();
	_parameter2Idx.clear();

	for (size_t i = 0; i < basis->parameterCount(); i++)
	{
		Parameter *p = basis->parameter(i);
		ResidueTorsion rt{};
		tmp_motions.push_back(make_mot(p, rt));
		torsions.push_back(rt);
		_parameters.push_back(p);
		_parameter2Idx[p] = i;
	}

	_motions = RTMotion::motions_from(torsions, tmp_motions);
}

void Route::prepareParameters()
{
	auto instance_for_param = [this](Parameter *const &param) -> Instance *
	{
		Atom *atom = param->owningAtom();

		for (const InstancePair &pair : _pairs)
		{
			if (pair.start->atomBelongsToInstance(atom))
			{
				return pair.start;
			}
		}
		
		return nullptr;
	};

	auto make_motion = [this, instance_for_param]
	(Parameter *const &param, ResidueTorsion &rt)
	{
		Instance *inst = instance_for_param(param);
		rt = ResidueTorsion(param);
		rt.attachToInstance(inst);
		
		// index may be in existing motions, or in the source torsions
		int mot_idx = _motions.indexOfHeader(rt);
		int src_idx = _source.indexOfHeader(rt);
		
		Motion mt = {WayPoints(_order, _randomPerturb), false, 0};

		if (mot_idx >= 0)
		{
			Motion &target = _motions.storage(mot_idx);
			mt = target;
			mt.wp.forceOrder(_order);
		}

		// if we have a more up-to-date torsion angle then we should use that,
		// but take into account 360 degree flips
		if (src_idx >= 0 && mot_idx < 0)
		{
			mt.angle = _source.storage(src_idx);
		}
		else if (src_idx >= 0 && mot_idx >= 0)
		{
			float torsion = _source.storage(src_idx);
			float last = mt.angle;
			while (torsion < last - 180.f) torsion += 360.f;
			while (torsion >= last + 180.f) torsion -= 360.f;

			mt.angle = torsion;
		}
		
		if (mt.angle != mt.angle)
		{
			mt.angle = 0; // usually due to mismatched sequence
		}
		
		return mt;
	};

	getParametersFromBasis(make_motion);
}

void Route::prepareDestination()
{
	if (_source.size() == 0 && _motions.size() == 0)
	{
		throw std::runtime_error("Raw destination not set, nor motions for "\
		                         "destination");
	}
	
	prepareParameters();
}

int Route::indexOfParameter(Parameter *t)
{
	if (_parameter2Idx.count(t) == 0)
	{
		return -1;
	}

	return _parameter2Idx.at(t);
}

float Route::getTorsionAngle(int i)
{
	return  motion(i).workingAngle();
}

std::vector<ResidueTorsion> Route::residueTorsions()
{
	std::vector<ResidueTorsion> rts;
	for (size_t i = 0; i < motionCount(); i++)
	{
		rts.push_back(residueTorsion(i));
	}

	return rts;
}

void Route::prepareResources()
{
	_resources.allocateMinimum(_threads);

	/* prepare separate sequences for main-chain only */
	_hydrogenFreeSequences = new BondSequenceHandler(_threads);

	for (const InstancePair &pair : _pairs)
	{
		AtomGroup *group = pair.start->currentAtoms();
		std::vector<AtomGroup *> subsets = group->connectedGroups();
		for (AtomGroup *subset : subsets)
		{
			Atom *anchor = subset->chosenAnchor();
			_resources.sequences->addAnchorExtension(anchor);
			_hydrogenFreeSequences->addAnchorExtension(anchor);
		}
	}

	_hydrogenFreeSequences->setIgnoreHydrogens(true);
	_hydrogenFreeSequences->setAtomFilter(rope::atom_is_not_hydrogen());
	_hydrogenFreeSequences->setup();
	_hydrogenFreeSequences->prepareSequences();

	_resources.sequences->setup();
	_resources.sequences->prepareSequences();
}

void Route::deleteHelpers()
{
	for (auto it = _helpers.begin(); it != _helpers.end(); it++)
	{
		it->second.pairwise.clear();
		it->second.separation.clear();
		delete it->second.et;
	}

	_helpers.clear();
}

void Route::installAllResidues()
{
	PairwiseDeviations *pd = _helpers[_resources.sequences].pairwise.acquire();
	_ids = pd->residues();
}

void setup_helpers(Route::Helpers &helpers, BondSequence *seq, 
                   float distance, bool multi, bool momentum = false)
{
	auto make_separation = [seq]()
	{
		return new Separation(seq);
	};

	helpers.separation = Resource<Separation>(make_separation);
	
	auto make_pairwise = [&helpers, seq, distance, multi, momentum]()
	{
		Separation *sep = multi ? helpers.separation.acquire() : nullptr;
		return new PairwiseDeviations(seq, distance, sep, momentum);
	};

	helpers.pairwise = Resource<PairwiseDeviations>(make_pairwise);
}

void Route::prepareEnergyTerms()
{
	deleteHelpers();

	bool multiInstance = _noncovs;

	{
		BondSequence *seq = _hydrogenFreeSequences->sequence();
		setup_helpers(_helpers[_hydrogenFreeSequences], 
		              seq, _maxMomentumDistance, multiInstance, true);
	}

	{
		BondSequence *seq = _resources.sequences->sequence();
		setup_helpers(_helpers[_resources.sequences], seq,
		              _maxClashDistance, multiInstance);
	}

	{
		auto lookup = [this](Parameter *t)
		{
			return indexOfParameter(t);
		};
		auto et = new EnergyTorsions(_resources.sequences->sequence(),
		                             motions(), lookup);
		_helpers[_resources.sequences].et = et;
	}

}

void Route::updateAtomFetches()
{
	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_hydrogenFreeSequences);
}

void Route::updateAtomFetch(BondSequenceHandler *const &handler)
{
	const std::vector<AtomBlock> &blocks = 
	handler->sequence()->blocks();

	CoordManager *manager = handler->manager();
	
	auto filter = [&blocks, this](const int &idx)
	{
		Atom *atom = blocks[idx].atom;
		return _selection.atomIsActive(atom);
	};

	manager->setAtomFetcher(AtomBlock::prepareMovingTargets(blocks,
	                                                        filter));
}

void Route::clearCustomisation()
{
	_finish = false;

	for (size_t i = 0; i < motionCount(); i++)
	{
		motion(i).wp = WayPoints(_order, _randomPerturb);
		motion(i).flip = false;
	}

//	_helpers[_resources.sequences].pairwise.reset();
//	_helpers[_hydrogenFreeSequences].pairwise.reset();
	
	setFirstJob();
	_repelCount = 0;
	_hash = ""; setHash();
}

void Route::setHash(const std::string &hash)
{
	if (_hash.length() == 0 && hash.length() > 0)
	{
		_hash = hash;
	}
	else if (_hash.length() == 0)
	{
		unsigned int rand_num = rand();
		char str[65];
		sprintf(str, "%x", rand_num);

		std::string hash; hash += str;
		_hash = hash;
		std::cout << "New hash: " << _hash << std::endl;
	}
}

void Route::calculate(Route *me)
{
	try
	{
		me->_calculating = true;
		me->doCalculations();
		me->_calculating = false;
	}
	catch (const std::runtime_error &error)
	{
		std::string *str = new std::string(error.what());
		me->sendResponse("error", str);
	}
}

int Route::paramIdxForAtom(Atom *const &atom)
{
	BondSequence *const &seq = _resources.sequences->sequence();
	std::vector<AtomBlock> &blocks = seq->blocks();

	int idx = -1;
	for (size_t i = 0; i < blocks.size(); i++)
	{
		if (blocks[i].atom == atom)
		{
			idx = i;
			break;
		}
	}
	
	if (idx < 0) return -1;
	
	for (size_t i = 0; i < blocks.size(); i++)
	{
		for (int j = 0; j < blocks[i].nBonds; j++)
		{
			int add = blocks[i].write_locs[j];
			if (i + add == idx)
			{
				return blocks[i].torsion_idx;
				int first = blocks[i].write_locs[0];
				return blocks[i + first].torsion_idx;
			}
		}
	}

	return -1;
}

