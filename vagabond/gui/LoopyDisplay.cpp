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
#include "LoopyDisplay.h"
#include "Polymer.h"

LoopyDisplay::LoopyDisplay(Scene *prev, Polymer *const &pol) : Display(prev),
_loopy(pol)
{
	_polymer = pol;
	_polymer->load();
	setOwnsAtoms(false);
	std::cout << "Polymer: " << pol << std::endl;
	_loopy.setResponder(this);
}

LoopyDisplay::~LoopyDisplay()
{
	_polymer->unload();

}

void LoopyDisplay::setup()
{
	AtomGroup *grp = _polymer->currentAtoms();
	loadAtoms(grp);
	mechanicsButton();
	Display::setup();

	TextButton *button = new TextButton("Begin build loop", 
	                                    this);
	button->setReturnTag("looping");
	button->resize(0.8);
	button->setCentre(0.5, 0.85);
	addObject(button);

}

void LoopyDisplay::buttonPressed(std::string tag, Button *button)
{
	if (tag == "looping")
	{
		if (_loopy.size() >= 1)
		{
			_polymer->load();
			_worker = new std::thread(&Loopy::operator(), &_loopy, 0);
			_polymer->unload();
		}
	}

	Display::buttonPressed(tag, button);
}
