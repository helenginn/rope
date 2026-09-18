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

#include "EntropyTableView.h"
#include "TableView.h"

#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/Entropy.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>

EntropyTableView::EntropyTableView(Scene *prev, struct EntropyForMatrix *entropy) : Scene(prev)
{
	_entropy = entropy;
}

EntropyTableView::~EntropyTableView()
{

}

void EntropyTableView::setup()
{
	addTitle("Entropy - Residue Contributions");
	{
		Text *t = new Text("Number of entries");
		t->setLeft(0.2, 0.3);
		addObject(t);
	}
	{
		std::string num = i_to_str();
		Text *t = new Text(num);
		t->setLeft(0.8, 0.3);
		addObject(t);
	}
}

void EntropyTableView::buttonPressed(std::string tag, Button *button)
{
Scene::buttonPressed(tag, button);
}

