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

	int _max = 0;

	// shoot() keeps every probe within this many hops of *some* shortest
	// path between its chosen endpoints, not just one arbitrary path.
	int _slack = 5;

	// how many times each probe has already turned up in a searched
	// (post-prune) chunk this subdivide() call, kept across successive
	// grow_clique() walks so shoot() can bias its end-node choice towards
	// under-sampled nodes instead of picking uniformly at random - see
	// shoot()'s own comment for why.
	std::map<Probe *, int> _nodeCounts;

	// the `samples` argument passed to the current subdivide() call, kept
	// here so shoot() can tell when every candidate endpoint it could pick
	// is already oversampled - see shoot()'s own comment for why.
	int _samples = 0;
};

#endif
