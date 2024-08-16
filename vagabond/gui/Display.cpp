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

#include "Display.h"
#include "GuiAtom.h"
#include "GuiRefls.h"
#include "GuiDensity.h"

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/FloatingText.h>

#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/grids/ArbitraryMap.h>
#include <vagabond/core/VisualPreferences.h>

#include <SDL2/SDL.h>
#include <iostream>

Display::Display(Scene *prev) : 
Scene(prev),
Mouse3D(prev),
IndexResponseView(prev)
{
	_alwaysOn = true;
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);
}

Display::~Display()
{
	for (DisplayUnit *unit : _units)
	{
		delete unit;
	}
	
	_units.clear();
}

void Display::stopGui()
{
	for (DisplayUnit *unit : _units)
	{
		unit->stopGui();
	}

}

void Display::fftButton()
{
	return;
	TextButton *text = new TextButton("Fourier", this);
	text->setRight(0.95, 0.25);
	text->setReturnTag("fft");
	addObject(text);

}

void Display::wedgeButtons()
{
	return;
	ImageButton *lemon = new ImageButton("assets/images/orange.png", this);
	lemon->resize(0.16);
	lemon->setCentre(0.9, 0.1);
	lemon->setReturnTag("half_wedge");
	addObject(lemon);
	_halfWedge = lemon;

	ImageButton *orange = new ImageButton("assets/images/lemon.png", this);
	orange->resize(0.16);
	orange->setCentre(0.9, 0.1);
	orange->setReturnTag("full_wedge");
	orange->setDisabled(true);
	_wedge = orange;
	addObject(orange);
}

void Display::mechanicsButton()
{
	return;
	ImageButton *mechanics = new ImageButton("assets/images/torsion.png", this);
	mechanics->resize(0.10);
	mechanics->setCentre(0.94, 0.24);
	mechanics->setReturnTag("mechanics");
	_mechanics = mechanics;
	addObject(mechanics);
}

void Display::densityButton()
{
	return;
	ImageButton *density = new ImageButton("assets/images/density.png", this);
	density->resize(0.15);
	density->setCentre(0.94, 0.12);
	density->setReturnTag("density");
	_density = density;
	addObject(density);
}

void Display::registerPosition(const glm::vec3 &pos)
{
	shiftToCentre(pos, 0);
}

void Display::tieButton()
{
	{
		TextButton *button = new TextButton("Tie up with bonds", 
		                                    this);
		button->setReturnTag("recalculate");
		button->resize(0.8);
		button->setCentre(0.5, 0.85);
		addObject(button);
	}
}

void Display::setup()
{
#ifndef __EMSCRIPTEN__
	if (_pingPong)
	{
		preparePingPongBuffers();
	}
#endif

//	IndexResponseView::setup();
}

void Display::clearColours()
{
	for (DisplayUnit *unit : _units)
	{
		AtomGroup *group = unit->atoms();
		group->do_op([](Atom *const &a) { a->clearColour(); });
	}
}

void Display::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		triggerResponse();
	}

	Scene::buttonPressed(tag, button);

	if (tag == "recalculate")
	{
		for (DisplayUnit *unit : _units)
		{
			AtomGroup *grp = unit->atoms();
			grp->recalculate();
		}
		static_cast<TextButton *>(button)->setReturnTag("refine_positions");
	}
	else if (tag == "refine_positions")
	{
		for (DisplayUnit *unit : _units)
		{
			unit->atoms()->refinePositions(false, true);
		}
	}
}

void Display::interactedWithNothing(bool left, bool hover)
{
	if (hover)
	{
		supplyFloatingText(nullptr);
	}
}

void Display::supplyFloatingText(FloatingText *text)
{
	if (_text == text) return;
	if (_text)
	{
		removeObject(_text);
		delete _text;
		_text = nullptr;
	}

	_text = text;
	addObject(_text);
}

void Display::focusOnResidue(int res)
{
	for (DisplayUnit *const &unit : _units)
	{
		AtomGroup *atoms = unit->atoms();
		Atom *chosen = atoms->atomByIdName({res}, "CA");
		if (!chosen)
		{
			chosen = atoms->atomByIdName({res}, "");
		}
		if (!chosen)
		{
			continue;
		}

		shiftToCentre(chosen->derivedPosition(), 5);
	}
}

void Display::keyPressEvent(SDL_Keycode pressed)
{
	if (pressed == SDLK_g)
	{
		TextEntry *te = new TextEntry("enter residue", this);
		te->setValidationType(TextEntry::Numeric);
		te->setReturnJob([this, te]()
		                 {
			                std::string scr = te->scratch();
			                int res = atoi(scr.c_str());
			                focusOnResidue(res);
			                removeObject(te);
		                 });
		te->setCentre(0.5, 0.3);
		addObject(te);
		
		addMainThreadJob([te]() { te->click(); });
	}

	Mouse2D::keyPressEvent(pressed);
}
