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

#include "ContactPoint.h"
#include "Playground.h"
#include "Positions.h"
#include "Mesh.h"
#include "Mab.h"
#include <vagabond/gui/elements/Icosahedron.h>

Playground::Playground(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

}

void Playground::showMesh(const Competition &comp)
{
	std::string antigen_name = comp.antigen;
	Antigen *antigen = _mab.antigens(antigen_name);

	Mesh *mesh = antigen->mesh();
	addObject(mesh);
	shiftToCentre(mesh->centroid(), 0);
	std::cout << "Mesh centre: " << mesh->centroid() << std::endl;

}

void Playground::showFiducials(const Competition &comp)
{
	_positions = new Positions(*_mab.antigens(comp.antigen), comp, _mab);
	_positions->loadAntibodiesInto(this);
}

void Playground::setup()
{
	Competition &comp = *_mab.competitions.begin();
	showMesh(comp);
	showFiducials(comp);
}
