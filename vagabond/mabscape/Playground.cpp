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
	std::string antigen_name = comp.antigen;
	for (Fiducial &fid : _mab.fiducials)
	{
		if (fid.antigen == antigen_name)
		{
			ContactPoint *cp = fid.contact;
			if (!cp)
			{
				continue;
			}
			
			glm::vec3 position = cp->reference();
			std::cout << "Position: " << position << std::endl;
			
			for (const glm::mat4x4 &transform : cp->transforms())
			{
				cp->Symmetry::applyTransform(transform, 
				                             Symmetry::next_pointer(&position));

				Icosahedron *ico = new Icosahedron();
				ico->triangulate();
				ico->setPosition(position);
				std::cout << "\tSymmetry: " << position << std::endl;
				addObject(ico);
			}
		}
	}
}

void Playground::setup()
{
	Competition &comp = *_mab.competitions.begin();
	showMesh(comp);
	showFiducials(comp);
}
