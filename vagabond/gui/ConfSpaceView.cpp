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

#include "ConfSpaceView.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/AskYesNo.h>

#include <vagabond/core/Entity.h>

ConfSpaceView::ConfSpaceView(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
}

void ConfSpaceView::askToFoldIn(int extra)
{
	std::string str = "Found " + i_to_str(extra) + " unrefined molecules.\n";
	str += "Refine torsion angles to atom positions\nnow?";
	AskYesNo *askyn = new AskYesNo(this, str, "fold_in", this);
	setModal(askyn);
}

void ConfSpaceView::setup()
{
	size_t extra = _entity->checkForUnrefinedMolecules();

	if (extra > 0)
	{
		askToFoldIn(extra);
	}
}


void ConfSpaceView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "fold_in_yes")
	{
		// refine extra molecules

		_entity->refineUnrefinedModels();
	}
	if (tag == "fold_in_yes" || tag == "fold_in_no")
	{
		// show conf space
	}

	Scene::buttonPressed(tag, button);
}
