// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "engine/MapTransferHandler.h"
#include "CompareDistances.h"
#include "BondSequence.h"
#include "engine/Task.h"
#include "Result.h"
#include <thread>

BondSequenceHandler::BondSequenceHandler(int totalSeq) : Handler()
{
	_totalSequences = totalSeq;
	_pools[SequenceIdle].setName("idle sequences");
	
	_manager = new CoordManager();
}

BondSequenceHandler::~BondSequenceHandler()
{
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		if (i == 0)
		{
			delete _sequences[i]->torsionBasis();
		}

		delete _sequences[i];
	}
	
	std::map<SequenceState, Pool<BondSequence *> >::iterator it;
}

void BondSequenceHandler::setup()
{
	prepareSequenceBlocks();
	torsionBasis()->prepare();
}

void BondSequenceHandler::prepareSequences()
{
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		_sequences[i]->reset();
		_sequences[i]->prepareForIdle();
	}
}

void BondSequenceHandler::prepareSequenceBlocks()
{
	BondSequence *sequence = new BondSequence(this);
	sequence->setSampleCount(_totalSamples);
	transferProperties(sequence);

	for (size_t j = 0; j < _atoms.size(); j++)
	{
		sequence->addToGraph(_atoms[j]);
	}

	if (_atoms.size() == 0)
	{
		sequence->makeTorsionBasis();
	}

	sequence->generateBlocks();
	sequence->multiplyUpBySampleCount();
	_elements = sequence->elementList();

	manager()->setAtomFetcherFromBlocks(sequence->blocks());
	_sequences.push_back(sequence);

	for (size_t i = 0; i < _totalSequences; i++)
	{
		BondSequence *copy = new BondSequence(*sequence);
		_sequences.push_back(copy);
	}
}

void BondSequenceHandler::addAnchorExtension(AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondSequenceHandler::signalToHandler(BondSequence *seq, SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	pool.pushObject(seq);
}

BondSequence *BondSequenceHandler::acquireSequenceOrNull()
{
	Pool<BondSequence *> &pool = _pools[SequenceIdle];
	BondSequence *seq = nullptr;
	
	pool.acquireObjectIfAvailable(seq);
	return seq;
}

TorsionBasis *BondSequenceHandler::torsionBasis() const
{
	if (_sequences.size() == 0)
	{
		return nullptr;
	}
	
	return _sequences[0]->torsionBasis();
}

void BondSequenceHandler::imposeDepthLimits(int min, int max, bool limit_max)
{
	for (BondSequence *sequence : _sequences)
	{
		sequence->reflagDepth(min, max, limit_max);
	}
}

int BondSequenceHandler::activeTorsions()
{
	return sequence(0)->activeTorsions();
}

void BondSequenceHandler::calculate(Flag::Calc flags,
                                    const std::vector<float> &parameters,
                                    BaseTask **first_task,
                                    CalcTask **final_hook,
                                    const CoordManager *specific_manager)
{
	const CoordManager *man = (specific_manager ? specific_manager : manager());
	rope::GetFloatFromCoordIdx toTorsions = man->defaultTorsionFetcher();
	calculate(flags, parameters, first_task, final_hook, man, toTorsions);
}

void BondSequenceHandler::calculate(Flag::Calc flags,
                                    const std::vector<float> &parameters,
                                    BaseTask **first_task,
                                    CalcTask **final_hook,
                                    const CoordManager *specific_manager,
                                    const rope::GetFloatFromCoordIdx &toTorsions)
{
	const CoordManager *man = (specific_manager ? specific_manager : manager());
	rope::GetListFromParameters transform = man->defaultCoordTransform();
	rope::IntToCoordGet paramToCoords = transform(parameters);
	rope::GetVec3FromCoordIdx coordsToPos = man->defaultAtomFetcher();

	auto grabSequence = [this](void *, bool *success) -> BondSequence *
	{
		BondSequence *seq = acquireSequenceOrNull();
		*success = (seq != nullptr);
		return seq;
	};

	auto calculateAtoms = [paramToCoords, toTorsions]
	(BondSequence *seq) -> BondSequence *
	{
		seq->calculateTorsions(paramToCoords, toTorsions);
		return seq;
	};

	auto targetAtoms = [paramToCoords, coordsToPos]
	(BondSequence *seq) -> BondSequence *
	{
		seq->calculateAtoms(paramToCoords, coordsToPos);
		return seq;
	};

	auto identity = [](BondSequence *seq) -> BondSequence *
	{
		return seq;
	};

	auto superposition = [](BondSequence *seq) -> BondSequence *
	{
		seq->superpose();
		return seq;
	};
	
	auto *grab = new FailableTask<void *, BondSequence *>(grabSequence, 
	                                                      "grab sequence");

	// do atom targets
	auto *get_targets = (flags & Flag::DoPositions ? 
	                     new CalcTask(targetAtoms, "target atoms") :
	                     nullptr);

	// do torsion targets
	auto *get_predicts = (flags & Flag::DoTorsions ? 
	                      new CalcTask(calculateAtoms, "calculate atoms") 
	                      : nullptr);

	auto *superpose = (flags & Flag::DoSuperpose ? 
	                   new CalcTask(superposition, "superpose")
	                   : new CalcTask(identity, "identity"));
	
	if (get_targets)
	{
		grab->follow_with(get_targets);
		get_targets->follow_with(superpose);
	}
	if (get_predicts)
	{
		grab->follow_with(get_predicts);
		get_predicts->follow_with(superpose);
	}
	if (!get_targets && !get_predicts)
	{
		grab->follow_with(superpose);
	}
	
	*final_hook = superpose;
	*first_task = grab;
}

Task<BondSequence *, void *> *BondSequenceHandler::letGo()
{
	auto letSequenceGo = [this](BondSequence *seq) -> void *
	{
		seq->setState(SequenceIdle);
		_pools[SequenceIdle].pushUnavailableObject(seq);
		return nullptr;
	};

	auto *letgo = new Task<BondSequence *, void *>(letSequenceGo,
	                                               "let sequence go");
	return letgo;
}

void BondSequenceHandler::positionsForMap(CalcTask *hook,
                                          Task<BondSequence *, void *> *letgo,
                                          std::map<std::string, GetEle> &eleTasks)
{
	for (auto it = _elements.begin(); it != _elements.end(); it++)
	{
		const std::string &ele = it->first;
		int num = it->second;

		auto grab_elements = 
		[ele, num](BondSequence *seq) -> std::vector<glm::vec3> *
		{
			std::vector<glm::vec3> *results = new std::vector<glm::vec3>();
			*results = seq->extractForMap(ele, num);
			return results;
		};

		auto *get = new Task<BondSequence *, std::vector<glm::vec3>*>
		(grab_elements, "grab positions for " + ele);
		
		hook->follow_with(get);
		get->must_complete_before(letgo);
		eleTasks[ele].get_pos = get;
	}

}

auto atom_list()
{
	return [](BondSequence *seq) -> AtomPosList *
	{
		AtomPosList *apl = new AtomPosList();
		seq->extractVector(*apl);
		return apl;
	};
};

Task<BondSequence *, void *> *
BondSequenceHandler::extract_compare_distances(Task<Result, void *> *submit_result,
                                               CompareDistances *compare,
                                               CalcTask *hook, bool show)
{
	Task<BondSequence *, void *> *letgo = letGo();
	hook->follow_with(letgo);
	auto *make_atom_list = new Task<BondSequence *, AtomPosList *>(atom_list(), 
	                                                               "atom list");

	hook->follow_with(make_atom_list);
	make_atom_list->must_complete_before(letgo);

	auto process_list = [compare, show](AtomPosList *list) -> AtomPosList *
	{
		compare->process(*list, show);
		return list;
	};
	
	auto *list_to_compare = 
	new Task<AtomPosList *, AtomPosList *>(process_list, "compare distances");
	
	make_atom_list->follow_with(list_to_compare);
	list_to_compare->follow_with(submit_result);

	return letgo;
}

Task<BondSequence *, void *> *
BondSequenceHandler::extract(Flag::Extract flags, 
                             Task<Result, void *> *submit_result,
                             CalcTask *hook,
                             Task<BondSequence *, Deviation> **dev,
                             Task<BondSequence *, AtomPosList *> **list,
                             Task<BondSequence *, AtomPosMap *> **map)
{
	Task<BondSequence *, void *> *letgo = letGo();

	auto calcdev = [](BondSequence *seq) -> Deviation
	{
		float dev = seq->calculateDeviations();
		return {dev};
	};
	
	auto atom_map = [](BondSequence *seq) -> AtomPosMap *
	{
		AtomPosMap tmp = seq->extractPositions();
		AtomPosMap *apm = new AtomPosMap(tmp);
		return apm;
	};
	
	hook->follow_with(letgo);

	if (flags & Flag::Deviation)
	{
		auto *deviation = new Task<BondSequence *, Deviation>(calcdev, "deviation");
		if (dev) {*dev = deviation;}
		hook->follow_with(deviation);
		deviation->must_complete_before(letgo);
		if (submit_result)
		{
			deviation->follow_with(submit_result);
		}
	}
	if (flags & Flag::AtomVector)
	{
		auto *make_atom_list = new Task<BondSequence *, AtomPosList *>(atom_list(), 
		                                                               "atom list");
		if (list) {*list = make_atom_list;}
		hook->follow_with(make_atom_list);
		make_atom_list->must_complete_before(letgo);
		if (submit_result)
		{
			make_atom_list->follow_with(submit_result);
		}
	}
	if (flags & Flag::AtomMap)
	{
		auto *make_atom_map = new Task<BondSequence *, AtomPosMap *>(atom_map, 
		                                                     "atom map");
		if (map) {*map = make_atom_map;}
		hook->follow_with(make_atom_map);
		make_atom_map->must_complete_before(letgo);
		if (submit_result)
		{
			make_atom_map->follow_with(submit_result);
		}
	}
	
	return letgo;
}

const Grapher &BondSequenceHandler::grapher() const
{
	const Grapher &g = _sequences.at(0)->grapher();
	return g;
}

ParamSet BondSequenceHandler::flaggedParameters()
{
	return _sequences.at(0)->flaggedParameters();
}
