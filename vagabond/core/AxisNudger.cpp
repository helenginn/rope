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

#include "AxisNudger.h"
#include "AtomGraph.h"
#include "Grapher.h"

class MetadataGroup;

template <class T>
class Cluster;

AxisNudger::AxisNudger(Refine::Info *info, int axis, 
                       Cluster<MetadataGroup> *cluster) : 
PlausibleRoute(info->molecule, cluster, info->axes.size())
{
	_pType = BondCalculator::PipelineAtomPositions;
	_torsionType = TorsionBasis::TypeSimple;
	_info = info;
	_idx = axis;
	
	std::vector<Refine::RTA> &rtas = info->axes[axis].angles;
	std::vector<Angular> angles = Refine::RTA::angulars_from_rts(rtas);
	setRawDestination(angles);
}

void AxisNudger::setup()
{
	Route::setup();
	prepareDestination();
	collectTorsionPairs();
}

BondTorsion *AxisNudger::otherTorsion(AtomGraph *ag)
{
	BondTorsion *pair = nullptr;
	Atom *parent = ag->parent;
	if (parent == nullptr)
	{
		return nullptr;
	}

	const Grapher &gr = grapher();
	AtomGraph *parentGraph = gr.graph(parent);

	BondTorsion *torsion = parentGraph->torsion;
	if (torsion == nullptr)
	{
		return nullptr;
	}

	if (torsion->coversMainChain() && !torsion->isPeptideBond())
	{
		return torsion;
	}
	
	if (ag->children.size() == 0)
	{
		return nullptr;
	}

	Atom *child = ag->children[0]->atom;
	AtomGraph *childrenGraph = gr.graph(child);
	torsion = childrenGraph->torsion;
	if (!torsion || (torsion->coversMainChain() && !torsion->isPeptideBond()))
	{
		return torsion;
	}
	
	return nullptr;
}

void AxisNudger::collectTorsionPairs()
{
	_pairs.clear();

	for (int i = 0; i < torsionCount(); i++)
	{
		BondTorsion *bt = torsion(i);
		float angle = getTorsionAngle(i);
		
		if (fabs(angle) < 2.f)
		{
			continue;
		}

		if (!bt->coversMainChain() || bt->isPeptideBond())
		{
			continue;
		}
		
		AtomGraph *ag = grapherForTorsionIndex(i);
		BondTorsion *pair = otherTorsion(ag);

		if (pair == nullptr)
		{
			continue;
		}
		
		_pairs.push_back({bt, pair});
	}
}

float AxisNudger::scoreForPoint(Point &point)
{
	clearPoints();
	addPoint(point);
	submitJobAndRetrieve(0, false);
	float contribution = deviation(0);
	return contribution;
}

void AxisNudger::run()
{
	for (size_t i = 0; i < 5; i++)
	{
		nudge();
	}

}

void AxisNudger::nudge()
{
	clearPoints();
	
	Point point = destination();
	float base = scoreForPoint(point);
	std::cout << "Start: " << base << std::endl;

	for (RamachandranPairs &pair : _pairs)
	{
		int idx_a = indexOfTorsion(pair.first);
		int idx_b = indexOfTorsion(pair.second);

		const float step = 0.5;
		for (float f = -step * 10; f <= +step * 10; f += step)
		{
			Point edit = point;
			edit[idx_a] += f;
			edit[idx_b] -= f;

			float score = scoreForPoint(edit);
			if (score < base)
			{
				point = edit;
				base = score;
			}
		}
	}
	
	std::cout << "Done: " << base << std::endl;
	setDestination(point);
}

void AxisNudger::addToInfo()
{
	for (size_t i = 0; i < destinationSize(); i++)
	{
		float f = destination(i);
		_info->axes[_idx].angles[i].angle = f;
	}

}
