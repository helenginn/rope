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

#ifndef __vagabond__Mesh__
#define __vagabond__Mesh__

#include <nlohmann/json.hpp>
#include <vagabond/utils/OpSet.h>
#include <vagabond/gui/elements/SimplePolygon.h>
#include <vagabond/core/grids/ArbitraryMap.h>
using nlohmann::json;

struct Antigen;
class AtomGroup;

class Mesh : public SimplePolygon
{
public:
	Mesh(Antigen &antigen);

	AtomGroup *atoms();

	void refine();
private:
	ArbitraryMap mappedAtoms();
	int removeHollows(ArbitraryMap &map);
	void growBorder(ArbitraryMap &map);
	void marchingCubes();
	Antigen &_antigen;

	AtomGroup *_atoms{};
	std::map<GLuint, OpSet<GLuint>> _connections;
	void adjustVertices();
};

#endif
