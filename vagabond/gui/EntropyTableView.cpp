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

#include <vagabond/core/Entity.h>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/paths/Entropy.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>

EntropyTableView::EntropyTableView(Scene *prev, Entity *entity, const std::vector<EntropyResults> &entropy) : ListView(prev)
{
    _entity = entity;
	_entropy = entropy;
}

EntropyTableView::~EntropyTableView()
{

}

void EntropyTableView::setup()
{
	addTitle("Entropy - Residue Contributions");
	{
		Text *t = new Text("Number of residues");
		t->setLeft(0.2, 0.3);
		addObject(t);
	}
	{
		std::string num = i_to_str(_entity->sequence()->size());
		Text *t = new Text(num);
		t->setLeft(0.8, 0.3);
		addObject(t);
	}

    ListView::setup();
}

size_t EntropyTableView::lineCount()
{
    return _entity->sequence()->size();
}

Renderable *EntropyTableView::getLine(int i)
{
    Box *b = new Box();

    const std::string desc = _entity->sequence()->residue(i)->code();

    double entRes = _entropy[i].entResidue.front();

    Text *res = new Text(desc);
    res->setLeft(0.0, 0.);
    b->addObject(res);

    Text *ent = new Text(std::to_string(entRes));
    ent->setRight(0.6,0.);
    b->addObject(ent);

	return b;
}

void EntropyTableView::buttonPressed(std::string tag, Button *button)
{
    Scene::buttonPressed(tag, button);
}

