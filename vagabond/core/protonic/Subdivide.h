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

private:
	Clique *_clique{};

	int _max = 0;

	// shoot() keeps every probe within this many hops of *some* shortest
	// path between its chosen endpoints, not just one arbitrary path.
	int _slack = 5;
};

#endif
