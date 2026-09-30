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

#include "AlignmentView.h"
#include "Mab.h"
#include "Mesh.h"
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Atom.h>
#include <vagabond/gui/DisplayUnit.h>
#include <vagabond/gui/elements/Text.h>

AlignmentView::AlignmentView(Scene *prev, Antigen &antigen, 
                             Fiducial &fiducial, const glm::mat4x4 &transform,
                             const std::string &warning)
: Scene(prev), Display(prev), _antigen(antigen), _fiducial(fiducial),
_transform(transform), _warning(warning)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);
}

void AlignmentView::setup()
{
	addTitle("Poor alignment: " + _fiducial.name);

	Text *text = new Text("Warning: " + _warning + ".\nThe antibody is shown "\
	                      "where this alignment puts it on the antigen mesh.");
	text->resize(0.6);
	text->setCentre(0.5, 0.17);
	addObject(text);

	// the mesh itself belongs to the antigen, so it is only borrowed
	_mesh = _antigen.mesh();
	addObject(_mesh);
	shiftToCentre(_mesh->centroid(), 0);

	// The antibody's model, put where the alignment says it goes. It is shown
	// first and moved afterwards, as the representations (ribbons and so on)
	// follow the atoms through the unit's watch, which is only there once the
	// unit has been displayed.
	_fiducial.model.load();
	AtomGroup *atoms = _fiducial.model.currentAtoms();

	_unit = new DisplayUnit(this);
	_unit->loadAtoms(atoms);
	_unit->displayAtoms();
	_unit->startWatch();
	addDisplayUnit(_unit);

	glm::mat4x4 transform = _transform;
	addMainThreadJob([atoms, transform]()
	{
		for (Atom *atom : atoms->atomVector())
		{
			glm::vec3 d = atom->derivedPosition();
			atom->setDerivedPosition(glm::vec3(transform * glm::vec4(d, 1.f)));
		}
	});
}

AlignmentView::~AlignmentView()
{
	if (_mesh)
	{
		removeObject(_mesh);
	}

	if (_unit)
	{
		removeDisplayUnit(_unit);
		_fiducial.model.unload();
	}
}
