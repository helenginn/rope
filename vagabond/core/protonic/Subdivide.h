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

#ifndef __vagabond__Subdivide__
#define __vagabond__Subdivide__

#include <vagabond/utils/OpSet.h>
#include <map>

class Probe;
class Clique;

class Subdivide
{
public:
	Subdivide(Clique *clique, int max);

	void subdivide(int samples = 3);
	void one();

	void shoot(OpSet<Probe *> &chunk);
	void prune(OpSet<Probe *> &chunk);
	static bool finish_ends(OpSet<Probe *> &chunk);
	static void finish_hbonds(OpSet<Probe *> &chunk);

private:
	Clique *_clique{};

	// shoot()'s own growth budget, in log2-penalty units rather than a
	// plain node count - see node_penalty()/shoot()'s own comments in
	// Subdivide.cpp for the per-node-type weights and why the same
	// numeric value carries over unconverted from the old node-count
	// meaning. finish_ends()/finish_hbonds() are NOT governed by this at
	// all - they run independently, uncapped, after shoot() finishes.
	int _max = 0;

	// shoot() keeps every probe within this much extra log2-penalty cost
	// of *some* shortest path between its chosen endpoints, not just one
	// arbitrary path - was a plain hop count before node_penalty()
	// weighting replaced _max as shoot()'s budget; may need retuning by
	// eye now that the unit has changed.
	int _slack = 5;

	// how many times each probe has already turned up in a searched
	// (post-prune) chunk this subdivide() call, kept across successive
	// grow_clique() walks so shoot() can bias its end-node choice towards
	// under-sampled nodes instead of picking uniformly at random - see
	// shoot()'s own comment for why.
	std::map<Probe *, int> _nodeCounts;
};

#endif
