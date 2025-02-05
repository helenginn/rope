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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/files/File.h>
#include "Untangle.h"
#include "Visual.h"
#include "Points.h"
#include "UntangleView.h"

UntangleView::UntangleView(Scene *prev)
: Scene(prev), Display(prev)
{
	_translation.z -= 50;
	_farSlab = 40;
	_slabbing = true;
	setPingPong(true);
}

void UntangleView::load(const std::string &filename)
{
	_filename = filename;
	
	File *file = File::loadUnknown(filename);
	File::Type type = file->cursoryLook();
	
	if (type & File::Geometry)
	{
		std::cout << "Inserting " << filename << " into geometry" << std::endl;
		_geometries.insert(filename);
		return;
	}
	else
	{
		std::cout << "Not geometry file" << std::endl;
	}

	_untangle = new Untangle(this, filename, _geometries);
	Visual *visual = new Visual(_untangle);
	addObject(visual);
	addObject(visual->points());
	addIndexResponder(visual->points());
	_visual = visual;
	
	_resi = _untangle->firstResidue();
}

void UntangleView::setup()
{
	addTitle("Untangle");
	IndexResponseView::setup();
	
	TextButton *tb = new TextButton("Save", this);
	tb->setReturnTag("save");
	tb->setRight(0.95, 0.1);
	addObject(tb);
}

void UntangleView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "save")
	{
		_untangle->save("output.pdb");
	}
}

void UntangleView::keyReleaseEvent(SDL_Keycode pressed)
{
	if (pressed == SDLK_SPACE)
	{
		_shiftPressed ? _resi-- : _resi++;

		_visual->focusOn(_resi);
	}
	Scene::keyReleaseEvent(pressed);
}

void UntangleView::recalculate()
{
	addMainThreadJob([this]()
	{
		if (_visual)
		{
			_visual->updateBonds();
			_visual->points()->repositionAtoms();
		}
	});
}
