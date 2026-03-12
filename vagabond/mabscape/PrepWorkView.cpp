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
#include "PrepWorkView.h"
#include "Mesh.h"
#include "Mab.h"
#include <vagabond/utils/DoJob.h>

PrepWorkView::PrepWorkView(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

}

void PrepWorkView::setup()
{
	addTitle("Preparing workspace");

	DoJob([this]() { prepareAntigens(_mab.antigens); });
}

void PrepWorkView::prepareAntigens(Antigens &antigens)
{
	for (Antigen &antigen : _mab.antigens)
	{
		setInformation("Meshing up antigen " + antigen.title);
		Mesh *mesh = antigen.mesh();

		addObject(mesh);
		shiftToCentre(mesh->centroid(), 0);

		mesh->refine();
		removeObject(mesh);

		for (Fiducial &fid : _mab.fiducials)
		{
			if (fid.antigen == antigen.title)
			{
				ContactPoint contact(fid, _mab.antigens);
				std::cout << std::endl;
			}
		}
	}
}
