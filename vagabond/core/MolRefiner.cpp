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

#include "MolRefiner.h"
#include "SimplexEngine.h"
#include "AtomGroup.h"
#include "Instance.h"
#include "ArbitraryMap.h"
#include "AtomMap.h"
#include "Result.h"
#include "Warp.h"

#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include "engine/Tasks.h"
#include "engine/Task.h"

MolRefiner::MolRefiner(ArbitraryMap *comparison, 
                       Refine::Info *info) :
StructureModification(), 
_sampler(info->samples, info->master_dims), 
_translate(info->master_dims)
{
	setInstance(info->instance);
	_threads = 5;
	
	std::cout << "MolRefiner for " << info->instance->id() << std::endl;

	_map = comparison;
	_info = info;

	_instance->load();
}

float confParams(int n)
{
	return n + n * (n + 1) / 2;
}

float transParams(int n)
{
	return (n + 1) * 3;
}


MolRefiner::~MolRefiner()
{
	_resources.tasks->wait();
	_instance->unload();
}

float MolRefiner::getResult(int *job_id)
{
	retrieveJobs();
	float res = RunsEngine::getResult(job_id);
	return res;
}

Result *MolRefiner::submitJobAndRetrieve(const std::vector<float> &all)
{
	submitJob(all);
	Result *r = _resources.calculator->acquireResult();
	return r;
}

void MolRefiner::submitJob(std::vector<float> all)
{
	if (all.size() == 0)
	{
		all = _best;
	}

	std::vector<float> simple;
	const int &n = _info->master_dims;
	simple.reserve(confParams(n));
	simple.insert(simple.begin(), all.begin(), all.begin() + confParams(n));

	std::vector<float> trans;
	trans.reserve(transParams(n));
	trans.insert(trans.begin(), all.begin() + confParams(n),
	             all.end() + confParams(n) + transParams(n));
	
	_translate.copyInParameters(trans);

	calculate(simple);
}

void MolRefiner::calculate(const std::vector<float> &params)
{
	_ticket++;
	BaseTask *first_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	calculator->holdHorses();
	BondSequenceHandler *const &sequences = _resources.sequences;
	MapTransferHandler *const &eleMaps = _resources.perElements;
	MapSumHandler *const &sums = _resources.summations;
	CorrelationHandler *const &correlation = _resources.correlations;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(_ticket);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions |
	                             Flag::DoSuperpose);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	CalcTask *final_hook = nullptr;

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, params, &first_hook, &final_hook);
	
	/* on top of this, apply translations */
	rope::IntToCoordGet raw = _sampler.rawCoordinates();
	rope::GetVec3FromIdx get = _translate.translate(raw);
	
	auto add_tr = [get](BondSequence *seq) -> BondSequence *
	{
		seq->addTranslation(get);
		return seq;
	};

	CalcTask *translate = new CalcTask(add_tr, "translate");
	
	final_hook->follow_with(translate);

	letgo = sequences->extract(gets, submit_result, translate);

	/* Prepare the scratch space for per-element map calculation */
	std::map<std::string, GetEle> eleTasks;

	/* positions coming out of sequence to prepare for per-element maps */
	sequences->positionsForMap(translate, letgo, eleTasks);

	/* updates the scratch space */
	eleMaps->extract(eleTasks);

	/* summation of per-element maps into final real-space map */
	Task<SegmentAddition, AtomMap *> *make_map = nullptr;
	sums->grab_map(eleTasks, submit_result, &make_map);

	/* correlation of real-space map against reference */
	Task<CorrelMapPair, Correlation> *correlate = nullptr;
	correlation->get_correlation(make_map, &correlate);

	/* pop correlation into result */
	correlate->follow_with(submit_result);

	/* first task to be initiated by tasks list */
	_resources.tasks->addTask(first_hook);

	calculator->releaseHorses();
}

int MolRefiner::sendJob(const std::vector<float> &all)
{
	submitJob(all);
	return _ticket;
}

void MolRefiner::retrieveJobs()
{
	BondCalculator *calc = _resources.calculator;
	while (true)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			break;
		}

		int t = r->ticket;

		r->transplantPositions();

		float cc = r->correlation;
		setScoreForTicket(t, -cc);

		r->destroy();
	}
}

size_t MolRefiner::parameterCount()
{
	int n = _info->master_dims;
	return confParams(n) + transParams(n);
}

void MolRefiner::runEngine()
{
	prepareResources();

	if (!_info->instance->hasSequence())
	{
		return;
	}

	if (_map == nullptr)
	{
		throw std::runtime_error("Map provided to refinement is null");
	}
	
	SimplexEngine *engine = new SimplexEngine(this);
	engine->setVerbose(true);
	engine->setStepSize(0.2);
	engine->start();
	
	_best = engine->bestResult();
}

void MolRefiner::changeDefaults(CoordManager *manager)
{
	rope::GetListFromParameters transform = [this](const std::vector<float> &all)
	{
		return _sampler.coordsFromParams(all);
	};

	rope::GetFloatFromCoordIdx fetchTorsion = [this](const Coord::Get &get, 
	                                                 const int &idx)
	{
		return _info->warp->torsionAnglesForCoord()(get, idx);
	};

	manager->setTorsionFetcher(fetchTorsion);
	manager->setDefaultCoordTransform(transform);
}

void MolRefiner::prepareResources()
{
	_resources.allocateMinimum(_threads);
	/* set up per-bond/atom calculation */
	Atom *anchor = _instance->currentAtoms()->chosenAnchor();
	BondSequenceHandler *sequences = _resources.sequences;
	sequences->setTotalSamples(_sampler.pointCount());
	sequences->addAnchorExtension(anchor);
	sequences->setup();
	sequences->prepareSequences();
	
	changeDefaults(sequences->manager());

	AtomGroup *group = _instance->currentAtoms();

	/* calculate transfer to per-element maps */
	MapTransferHandler *perEles = new MapTransferHandler(sequences->elementList(), 
	                                                     _threads);
	perEles->supplyAtomGroup(group->atomVector());
	perEles->setup();
	_resources.perElements = perEles;
	
	/* summation of all element maps into one */
	MapSumHandler *sums = new MapSumHandler(_threads, perEles->segment(0));
	sums->setup();
	_resources.summations = sums;

	/* correlation of summed density map against reference */
	CorrelationHandler *cc = new CorrelationHandler(_map, sums->templateMap(),
	                                                _threads);
	cc->setup();
	_resources.correlations = cc;
}
