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

#include "Toolkit.h"
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/ModelManager.h>

Toolkit::Toolkit(Scene *scene) : ImageButton("assets/images/tools.png", scene)
{
	resize(0.12);
	setCentre(0.92, 0.12);
	addAltTag("tools");
	
	_scene = scene;
}

void Toolkit::click(bool left)
{
	Menu *m = new Menu(_scene, this);
	m->setReturnTag("tools");
	TextButton *tb1 = m->addOption("automodel");
	TextButton *tb2 = m->addOption("rescan");
	m->setup(this);
	tb1->addAltTag("automatic models from PDBs\nscan for defined entities");
	tb2->addAltTag("rescan for entities\nonly in existing models");

	_scene->setModal(m);

	Button::click();
}

void Toolkit::prepareProgress(int ticks, std::string text)
{
	VagWindow::window()->prepareProgressBar(ticks, text);
}

void Toolkit::buttonPressed(std::string tag, Button *button)
{
	if (tag == "automodel")
	{
		FileManager *fm = Environment::fileManager();
		fm->setFilterType(File::MacroAtoms);
		std::vector<std::string> list = fm->filteredList();
		prepareProgress(list.size(), "Automodelling...");

		VagWindow::addJob("automodel");

		return;
	}
	else if (tag == "rescan")
	{
		ModelManager *mm = Environment::modelManager();
		prepareProgress(mm->objectCount(), "Rescanning models...");

		VagWindow::addJob("rescan");

		return;
	}
}
