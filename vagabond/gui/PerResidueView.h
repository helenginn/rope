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

#ifndef __vagabond__PerResidueView__
#define __vagabond__PerResidueView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/RotamerOccupancy.h>

#include <memory>
#include <vector>

class Entity;
class Graph;
class Metadata;
class Residue;
class SequenceSlider;

class PerResidueView : public Scene
{
public:
	/** @param md metadata source used to fill the x axis of the graph
	 * @param header metadata field chosen for the x axis; may be empty,
	 * in which case instances are just plotted in an arbitrary order */
	PerResidueView(Scene *prev, Entity *entity,
	               std::shared_ptr<RotamerOccupancy> rota,
	               Metadata *md, std::string header);

	virtual void setup();
private:
	Entity *_entity = nullptr;
	std::shared_ptr<RotamerOccupancy> _rota;
	Metadata *_md = nullptr;
	std::string _header;

	SequenceSlider *_slider = nullptr;
	Graph *_graph = nullptr;
	Residue *_selectedResidue = nullptr;

	std::vector<RotamerOccupancy::RotamerBucket> _buckets;

	void onResidueSelected(Residue *r);
	void graphForAll();
	void addBucketPoints(Graph *graph, int series,
	                     const RotamerOccupancy::RotamerBucket &bucket);
	Graph *newGraph();
};

#endif
