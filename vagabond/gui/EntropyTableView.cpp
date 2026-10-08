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

#include <fstream>
#include <iostream>

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
		t->setLeft(0.2, 0.2);
		addObject(t);
	}
	{
		std::string num = i_to_str(lineCount());
		Text *t = new Text(num);
		t->setLeft(0.8, 0.2);
		addObject(t);
	}

    {
        TextButton *tb = new TextButton("Export CSV", this);
        tb->setLeft(0.9, 0.1);
        tb->setReturnTag("export");
        addObject(tb);
    }

    ListView::setup();
}

size_t EntropyTableView::lineCount()
{
    return _entropy[3].entResidue.size();
}

Renderable *EntropyTableView::getLine(int i)
{
    Box *b = new Box();

    const std::string desc = _entropy[0].resName[i];
    double entRes = 0.0;

	for (size_t t = 0; t < _entropy.size(); t++)
    {
		entRes += _entropy[t].entResidue[i];
	}

    Text *res = new Text(desc);
    res->setLeft(0.0, 0.);
    b->addObject(res);

    Text *ent = new Text(std::to_string(entRes/_entropy.size()));
    ent->setRight(0.6,0.);
    b->addObject(ent);

	return b;
}

void EntropyTableView::buttonPressed(std::string tag, Button *button)
{
    if(tag == "export")
    {
       std::ofstream outputPerRes("per_residue.csv");

       for (int i = 0; i < _entity->sequence()->size(); i++)
       {
           outputPerRes << _entity->sequence()->residue(i)->code() << "\t" << _entropy[i].entResidue.front() << std::endl;

       } 
    }

    ListView::buttonPressed(tag, button);
}

