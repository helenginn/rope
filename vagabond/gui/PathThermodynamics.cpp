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
#include <vagabond/core/PathEntropy.h>
#include <nlohmann/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/BadChoice.h>
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

	{
		Text *t = new Text("Calculate single-structure entropy");
		t->setLeft(0.2, top);
		addObject(t);		
	}

	{
		TextButton *t = new TextButton("Calculate (no MIST)", this);
		t->setLeft(0.2, 0.4);
		t->setReturnTag("calc_indep");
		addObject(t);
	}	

	{
		TextButton *t = new TextButton("Calculate (MIST)", this);
		t->setLeft(0.2, 0.5);
		t->setReturnTag("calc_mist");
		addObject(t);
	}
}

void PathThermodynamics::buttonPressed(std::string tag, Button *button)
{

    deleteTemps();

    struct FlagParameters flagPar = _pathEntropy->initFlagPar();

	if (tag == "calc_indep")
	{
		try
		{
		   checkPathNum(flagPar.n);
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, err.what());
            bc->setDismissible(true);
			this->setModal(bc);
            return;
		}

		std::string str = "Choose number of paths (\"frames\") to utilise";
		
		ChooseRange *cr = new ChooseRange(this, str, "choose_paths", this);
		cr->setDefault(1);
		cr->setRange(flagPar.n, _paths.size(), (_paths.size()-flagPar.n));

		auto respondToVal = [this, flagPar](float min, float max)
		{
			_numPaths = lrint(min);
			
			std::vector<TorsRes4NN*> tors_res = _pathEntropy->getAtomsAndResidues(_numPaths, _paths);

			_entropy = _pathEntropy->calculate_entropy_independent(_numPaths, flagPar, tors_res);

			std::string str = "Total: " +  std::to_string(_entropy->totalEntropy) + " (R units)\n" + "Per residue: " + std::to_string(_entropy->totalEntropy/tors_res.size()) + " (R units)";
			delete(_entropy);	
			displayEntropy(str);
		};

		cr->setReturn(respondToVal);
		setModal(cr);

	}

	if (tag == "calc_mist")
	{	
		try
		{
		   checkPathNum(flagPar.n);
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, err.what());
            bc->setDismissible(true);
            this->setModal(bc);
            return;
		}

		std::string str = "Choose number of paths (\"frames\") to utilise";
		
		ChooseRange *cr = new ChooseRange(this, str, "choose_paths", this);
		cr->setDefault(flagPar.n);
		cr->setRange(flagPar.n, _paths.size(), (_paths.size()-flagPar.n));

		auto respondToVal = [this, flagPar](float min, float max)
		{
			_numPaths = lrint(min);
			
			std::vector<TorsRes4NN*> tors_res = _pathEntropy->getAtomsAndResidues(_numPaths, _paths);

			_entropy = _pathEntropy->calculate_entropy_mi(_numPaths, flagPar, tors_res);

			std::string str = "Total: " +  std::to_string(_entropy->totalEntropy) + " (R units)\n" + "Per residue: " + std::to_string(_entropy->totalEntropy/tors_res.size()) + " (R units)";
			delete(_entropy);	
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
	t->setLeft(0.2, 0.8);
	addTempObject(t);
}

void PathThermodynamics::checkPathNum(int nearestNeighbours)
{
    if (_paths.size() < nearestNeighbours)
    {
        throw std::runtime_error("Desired nearest neighbour value exceeds total number of paths.");
    }
}

void PathThermodynamics::refresh()
{
	Scene::refresh();
}
