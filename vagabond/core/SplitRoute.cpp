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

#include "SplitRoute.h"
#include "Molecule.h"
#include "AtomGroup.h"
#include "ShortCalculator.h"
#include "BondSequence.h"
#include "Grapher.h"
#include "ShortRoute.h"

SplitRoute::SplitRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims)
: PlausibleRoute(mol, cluster, dims)
{
	_threads = 2;
	_maximumCycles = 20;
}

void SplitRoute::setup()
{
	PlausibleRoute::setup();
	prepareShortRoutes();
	
	submitJob(0);

	float begin = momentumScore(_nudgeCount);
	if (begin < 1.f)
	{
		_disable = true;
	}
}

void SplitRoute::findAtomSequence()
{
	const Grapher &g = grapher();

	AtomGraph *gr = g.graph(0);

	while (gr != nullptr)
	{
		Atom *anchor = gr->atom;
		BondTorsion *t = gr->torsion;

		int idx = indexOfTorsion(t);
		gr = g.deepestChild(gr);
		float mag = fabs(destination(idx));

		_atoms.push_back({anchor, gr, mag, 1});
	}
}

void SplitRoute::addTorsionIndices(std::vector<int> &idxs, AtomGraph *gr)
{
	const Grapher &g = grapher();
	AtomGraph *curr = gr;
	if (!curr)
	{
		return;
	}

	for (size_t i = 0; i < half_quiet + 1; i++)
	{
		if (curr->parent == nullptr)
		{
			break;
		}

		curr = g.graph(curr->parent);
	}

	for (size_t i = 0; i < quiet_size + 1; i++)
	{
		BondTorsion *bt = curr->torsion;
		int idx = indexOfTorsion(bt);
		if (idx >= 0)
		{
			idxs.push_back(idx);
		}
		
		curr = g.deepestChild(curr);
		
		if (curr == nullptr)
		{
			break;
		}
	}
}

bool SplitRoute::optimiseConnections()
{
	startTicker("Optimising connections", _atoms.size());

	bool result = false;
	_maxJobRuns = 60;
	for (size_t i = 1; i < _atoms.size(); i++)
	{
		clickTicker();

		if (Route::_finish)
		{
			return false;
		}

		AtomGraph *g = _atoms[i].graph;

		int idx = indexOfTorsion(g->torsion);
		if (idx < 0)
		{
			continue;
		}

		std::vector<int> idxs;
//		idxs = getTorsionSequence(idx, 6, false, 1.f);
		addTorsionIndices(idxs, g);
		if (idxs.size())
		{
			result |= simplexCycle(idxs);
		}
	}

	_maxJobRuns = 6;
	finishTicker();
	return result;
}

void SplitRoute::calculateFirstAnchors()
{
	std::vector<AtomInt> tmp;

	for (size_t i = 0; i < _atoms.size(); i++)
	{
		int count = 0;
		int j = 0;
		
		for (j = i + 1; j < _atoms.size(); j++)
		{
			count++;
			
			if (count < _count)
			{
				continue;
			}
			
			float total_mag = 0;
			for (size_t k = j; k < j + quiet_size && k < _atoms.size(); k++)
			{
				total_mag += _atoms[k].torsion;
			}
			
			if (total_mag < 20)
			{
				break;
			}
		}
		
		j += half_quiet;
		count = j - i - 1;
		AtomInt anchor{_atoms[i].atom, _atoms[i].graph, 0, count};
		tmp.push_back(anchor);
		
		i = j;
	}

	_atoms = tmp;

	for (AtomInt &ai : _atoms)
	{
		std::cout << "Anchor: " << ai.atom->desc() << " length " << ai.count 
		<< std::endl;
	}
}

bool SplitRoute::collapseShorts()
{
	if (_atoms.size() <= 1)
	{
		return false;
	}

	int shortest_collapse = INT_MAX;
	int idx = 0;

	for (size_t i = 0; i < _atoms.size() - 1; i++)
	{
		int mine = _atoms[i].count;
		int next = _atoms[i + 1].count;
		
		if (next + mine < shortest_collapse)
		{
			shortest_collapse = next + mine;
			idx = i;
		}
	}
	
	/* merge idx and idx + 1 */
	_atoms[idx].count += _atoms[idx + 1].count;
	_atoms.erase(_atoms.begin() + idx + 1);
	
	std::cout << "Total anchors: " << _atoms.size() << std::endl;

	return true;
}

void SplitRoute::prepareShortRoutes()
{
	findAtomSequence();
	calculateFirstAnchors();
}

void SplitRoute::makeShorts()
{
	for (AtomInt ac : _atoms)
	{
		ShortRoute *sr = new ShortRoute(_molecule, _cluster, Route::_dims);
		sr->setAtom(ac.atom, ac.count);
		sr->setDestination(_rawDest);
		sr->setup();
		sr->extractWayPoints(this);

		_splits.push_back(sr);
		_pool.pushObject(sr);
	}
}

void SplitRoute::prepareThreads()
{
	for (size_t i = 0; i < 4; i++)
	{
		/* several calculators */
		ShortCalculator *worker = new ShortCalculator(this);
		std::thread *thr = new std::thread(&ShortCalculator::start, worker);

		_pool.threads.push_back(thr);
		_pool.workers.push_back(worker);
	}
}

void SplitRoute::doShortRoutes()
{
	return;
	if (_disable)
	{
//		return;
	}

	makeShorts();
	prepareThreads();
	
	std::string label = "Short routes (" + std::to_string(_splits.size()) + ")";
	startTicker(label, _splits.size());

	_pool.joinThreads();
	
	finishTicker();
		
	for (ShortRoute *sr : _splits)
	{
		extractWayPoints(sr);
		delete sr;
	}
	
	_splits.clear();
	_pool.cleanup();
}

void SplitRoute::cycle()
{
	_magnitudeThreshold = 90;

	if (Route::_finish)
	{
		return;
	}

	setTargets();

	switch (_stage)
	{
		case FlippingTorsions:
		flipTorsionCycles();
		_stage = OptimisingConnections;
		break;
		
		case OptimisingConnections:
		optimiseConnections();
		_stage = DoingShorts;
		break;
		
		case DoingShorts:
		doShortRoutes();
		_stage = NudgingWaypoints;
		break;
		
		case NudgingWaypoints:
		nudgeWayPointCycles();
		_stage = FlippingTorsions;
		break;
		
		default:
		break;
	}
}

void SplitRoute::doCalculations()
{
	while (true)
	{
		if (Route::_finish)
		{
			break;
		}

		float begin = momentumScore(_nudgeCount);
		std::cout << "Begin: " << begin << std::endl;
		if (begin > 3)
		{
			_disable = false;
		}
		postScore(begin);

		cycle();

		float end = momentumScore(_nudgeCount);
		std::cout << "End: " << end << std::endl;
		postScore(end);
		
		if (end > begin * 0.99)
		{
//			_disable = true;
//			_maximumCycles = 1000;
//			_minimumMagnitude = 0.5;
		}

	}

	finishTicker();
	Route::_finish = false;
	prepareForAnalysis();
}

void SplitRoute::finishRoute()
{
	Route::finishRoute();
	
	for (ShortRoute *sr : _splits)
	{
		sr->finishRoute();
	}
}


ShortRoute *SplitRoute::acquireRoute()
{
	if (Route::_finish)
	{
		return nullptr;
	}

	ShortRoute *sr = nullptr;
	bool finish = true;
	_pool.acquireObject(sr, finish);
	
	return sr;
}

