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

#ifndef __vagabond__AtomWarp__
#define __vagabond__AtomWarp__

#include <vagabond/utils/AcquireCoord.h>

class PositionalCluster;
class TorsionCluster;
class RAMovement;
class Instance;
class Atom;

class AtomWarp
{
public:
	AtomWarp(std::vector<Instance *> instances, Instance *reference);

	std::function<Vec3s(const Coord::Get &get)> 
	mappedMotions(int n, const std::vector<Atom *> &order);
private:
	std::vector<RAMovement> allMotions(int n);

	TorsionCluster *_tCluster = nullptr;
	Instance *_reference = nullptr;
};

#endif
