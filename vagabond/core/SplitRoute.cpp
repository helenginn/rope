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
#include "Polymer.h"
#include "AtomGroup.h"
#include "BondSequence.h"
#include "Grapher.h"

SplitRoute::SplitRoute(Instance *inst, Cluster<MetadataGroup> *cluster, int dims)
: PlausibleRoute(inst, cluster, dims)
{
	_threads = 4;
	_maximumCycles = 5;
}

void SplitRoute::setup()
{
	PlausibleRoute::setup();
	prepareShortRoutes();
	
	submitJob(0);
}

void SplitRoute::findAtomSequence()
{
	const Grapher &g = grapher();

	AtomGraph *gr = g.graph(0);

	while (gr != nullptr)
	{
		Atom *anchor = gr->atom;
		BondTorsion *t = gr->torsion;

		int idx = indexOfParameter(t);
		gr = g.deepestChild(gr);
		if (idx > 0)
		{
			float mag = fabs(destination(idx));
			_atoms.push_back({anchor, gr, mag, 1});
		}
	}
}

void SplitRoute::addTorsionIndices(std::vector<int> &idxs, AtomGraph *gr)
{
	if (!gr)
	{
		return;
	}

	bool found = incrementToAtomGraph(gr);
	
	if (!found)
	{
		return;
	}

	AtomGraph *curr = gr;

	const Grapher &g = grapher();

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
		int idx = indexOfParameter(bt);
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
	return false;
	startTicker("Optimising connections", _atoms.size());

	bool result = false;
	for (size_t i = 1; i < _atoms.size(); i++)
	{
		clickTicker();

		if (Route::_finish)
		{
			return false;
		}

		AtomGraph *g = _atoms[i].graph;

		int idx = indexOfParameter(g->torsion);
		if (idx < 0)
		{
			continue;
		}

		std::vector<int> idxs;
		addTorsionIndices(idxs, g);
		if (idxs.size())
		{
			result |= simplexCycle(idxs);
		}
	}

	finishTicker();
	return false;
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
}

void SplitRoute::prepareShortRoutes()
{
	findAtomSequence();
	calculateFirstAnchors();
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
	int stages = 0;
	printWaypoints();

	while (true)
	{
		if (Route::_finish)
		{
			break;
		}
		
		float begin = routeScore(_nudgeCount);
		postScore(begin);

		cycle();

		float end = routeScore(_nudgeCount);
		postScore(end);

		if (Route::_finish)
		{
			break;
		}
		
		if (stages % 3 == 2 && end / begin > 0.95 && splitCount() < 2)
		{
			splitWaypoints();
		}
		
		stages++;
		
		if (stages == cycles())
		{
			break;
		}
	}

	finishTicker();
	Route::_finish = false;
	prepareForAnalysis();
}

