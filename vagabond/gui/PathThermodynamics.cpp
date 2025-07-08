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

#include <string>
#include "PathThermodynamics.h"
#include <vagabond/core/PathGroup.h>
#include <vagabond/core/Path.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/PathEntropy.h>
#include <nlohmann/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskForText.h>

int PathThermodynamics::_numPaths = 0;

PathThermodynamics::PathThermodynamics(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths) : Scene(prev)
{
	_entity = entity;
	_paths = paths;

	_pathEntropy = new PathEntropy();
}

PathThermodynamics::~PathThermodynamics()
{
	delete _pathEntropy;
}

void PathThermodynamics::addTypeButtons()
{
	TickBoxes *tb = new TickBoxes(this, this);
	tb->addOption("Use nearest neighbour algorithm", "nearest_neighbour");
	tb->arrange(0.2, 0.3, 1.0, 0.5);
}

void PathThermodynamics::setup()
{
	addTitle("Calculate path thermodynamics");
	float top = 0.3;

	if (_paths.size())
	{
		//PathGroup &group = _paths[0];
		//const std::string mod_id = group[0]->startInstance()->model_id();
	}


	{
		Text *t = new Text("Calculate single-structure entropy");
		t->setLeft(0.2, top);
		addObject(t);		
	}

	{
		TextButton *t = new TextButton("Calculate", this);
		t->setLeft(0.2, 0.4);
		t->setReturnTag("calc_indep");
		addObject(t);
	}
}

void PathThermodynamics::buttonPressed(std::string tag, Button *button)
{
	if (tag == "calc_indep")
	{	
		deleteTemps();

		std::string str = "Choose number of paths (\"frames\") to utilise";
		ChooseRange *cr = new ChooseRange(this, str, "choose_paths", this);
		cr->setDefault(1);
		cr->setRange(1, _paths.size(), _paths.size()-1);

		auto respondToVal = [this](float min, float max)
		{
			_numPaths = lrint(min);
			
			Sequence *seq = _entity->sequence();

			const std::string mod_id = _paths[0].front()->startInstance()->model_id();

			_pathEntropy->init_flag_par();	
			
            Tors_res4nn* tors_res = _pathEntropy->get_atoms_and_residues(_numPaths, _paths, seq);

			_pathEntropy->calculate_entropy_independent(_numPaths, seq, tors_res, _entropy);

			for (int i = 0; i < seq->size(); i++)
			{

				std::cout << _entropy->h1lm[i] << std::endl;
			
			}
			std::string str = "Total: " +  std::to_string(*(double *)_entropy->total) + " (R units)\n" + "Per residue: " + std::to_string(*(double *)_entropy->total/seq->size()) + " (R units)";
			
			displayEntropy(str);
	
		};

		cr->setReturn(respondToVal);
		setModal(cr);

	}

	if (tag == "choose_paths")
	{
	
		ChooseRange *cr = static_cast<ChooseRange *>(button->returnObject());
		float num = cr->max();
		_numPaths = lrint(num);
		refresh();
	}

	if (tag == "divisions")
	{
		AskForText *aft = new AskForText(this, "How many samples along the paths?", "samples", this, TextEntry::Numeric);
		setModal(aft);
	}

	Scene::buttonPressed(tag, button);
}

void PathThermodynamics::displayEntropy(std::string str)
{
	std::cout << "entropy calculated" << std::endl;
	Text *t = new Text(str);
	t->setLeft(0.2, 0.6);
	addTempObject(t);
}

void PathThermodynamics::refresh()
{
	Scene::refresh();
}
