// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "PerResidueView.h"
#include "SequenceSlider.h"
#include "Graph.h"

#include <vagabond/core/Entity.h>
#include <vagabond/core/IndexedSequence.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/RotamerOccupancy.h>

#include <set>

PerResidueView::PerResidueView(Scene *prev, Entity *entity,
                                std::shared_ptr<RotamerOccupancy> rota,
                                Metadata *md, std::string header)
: Scene(prev), _entity(entity), _rota(rota), _md(md), _header(header)
{

}

void PerResidueView::onResidueSelected(Residue *r)
{
	_selectedResidue = r;
	_buckets = _rota->bucketAltConfs(r, _md, _header);

	if (_buckets.size())
	{
		graphForAll();
	}
}

Graph *PerResidueView::newGraph()
{
	if (_graph)
	{
		deleteLater(_graph);
		_graph = nullptr;
	}

	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;
	graph->setRange('y', 0, 1);
	graph->setAxisLabel('y', "Occupancy");
	graph->setAxisLabel('x', _header.length() ? _header : "Instance");

	return graph;
}

void PerResidueView::addBucketPoints(Graph *graph, int series,
                                     const RotamerOccupancy::RotamerBucket &bucket)
{
	int i = 0;
	for (const RotamerOccupancy::AltConfPoint &pt : bucket.points)
	{
		float occ = _rota->altConfOccupancy(pt.instance, _selectedResidue,
		                                    pt.label);

		float x = (float)i;
		if (_md && _header.length())
		{
			Metadata::KeyValues kv = pt.instance->metadata(_md);
			auto it = kv.find(_header);
			if (it != kv.end())
			{
				x = it->second.number();
			}
		}

		graph->addPoint(series, x, occ, pt.instance->id());
		i++;
	}
}

// cycled per bucket (series) so a bucket's scatter points and connecting
// line share a colour distinct from its neighbours
static const glm::vec3 BucketPalette[] = {
	{0.85f, 0.20f, 0.20f}, {0.20f, 0.45f, 0.85f}, {0.20f, 0.70f, 0.30f},
	{0.85f, 0.55f, 0.10f}, {0.55f, 0.25f, 0.75f}, {0.10f, 0.65f, 0.65f},
	{0.75f, 0.20f, 0.55f}, {0.45f, 0.45f, 0.15f},
};
const int BucketPaletteSize = sizeof(BucketPalette) / sizeof(glm::vec3);

void PerResidueView::graphForAll()
{
	Graph *graph = newGraph();
	graph->connectPoints = true;

	for (size_t i = 0; i < _buckets.size(); i++)
	{
		graph->setSeriesColour((int)i, BucketPalette[i % BucketPaletteSize]);
		addBucketPoints(graph, (int)i, _buckets[i]);
	}

	graph->setup(0.4, 0.4);
	graph->addToGraphPosition(0.68, 0.4);
	addTempObject(graph);
	_graph = graph;
}

void PerResidueView::setup()
{
	addTitle("Per-residue Occupancy");

	IndexedSequence *sequence = _entity->sequence();

	SequenceSlider *slider = new SequenceSlider(sequence);
	slider->setBounds(0.1, 0.75, 0.9, 0.88, 0.93);

	auto onResidue = [this](Residue *r)
	{
		deleteTemps();
		_graph = nullptr;
		onResidueSelected(r);
	};

	slider->setReturnJob(onResidue);
	slider->setup();

	_rota->ensureModelsLoaded();

	std::set<Residue *> enabled;
	for (size_t i = 0; i < sequence->entryCount(); i++)
	{
		if (!sequence->hasResidue(0, i))
		{
			continue;
		}

		Residue *r = sequence->residue(0, i);
		if (_rota->hasAltConformers(r))
		{
			enabled.insert(r);
		}
	}

	_rota->unloadModels();

	slider->setEnabledResidues(enabled);

	addObject(slider);
	_slider = slider;
}
