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

#ifndef __vagabond__SplitRoute__
#define __vagabond__SplitRoute__

#include "Handler.h"
#include "PlausibleRoute.h"

class ShortRoute;
struct AtomGraph;

class SplitRoute : public PlausibleRoute, public Handler
{
public:
	SplitRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims);

	virtual void setup();
private:
	void cycle();
	void makeShorts();
	void prepareThreads();
	bool collapseShorts();
	void prepareShortRoutes();
	void findAtomSequence();
	void calculateFirstAnchors();
	bool optimiseConnections();
	virtual void doCalculations();

	std::vector<ShortRoute *> _splits;
	void addTorsionIndices(std::vector<int> &idxs, AtomGraph *gr);
	
	struct AtomInt
	{
		Atom *atom;
		AtomGraph *graph;
		float torsion;
		int count;
	};

	std::vector<AtomInt> _atoms;
	int _count = 20;
	bool _disable = false;

	const int half_quiet = 3;
	const int quiet_size = 6;
	
	enum Stage
	{
		FlippingTorsions,
		DoingShorts,
		OptimisingConnections,
		NudgingWaypoints,
	};
	
	Stage _stage = FlippingTorsions;
	
	Pool<ShortRoute *> _pool;
};

#endif
