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


#include "PathThermodynamics.h"
#include "CandidateView.h"
#include <vagabond/core/Entity.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/PathEntropy.h>
#include <nlohmann/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/BadChoice.h>

PathThermodynamics::PathThermodynamics(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
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
		TextButton *t = new TextButton("Calculate", this);
		t->setLeft(0.2, 0.4);
		t->setReturnTag("calc_indep");
		addObject(t);
	}

	refresh();
}

void PathThermodynamics::buttonPressed(std::string tag, Button *button)
{
	if (tag == "calc_indep")
	{
		struct Flag_par flag_par;
		struct Entropy entropy;

		PathEntropy path_entropy;

		path_entropy.get_atoms_and_residues(&model_id);
	
		path_entropy.init_flag_par(&flag_par);
		std::cout << "flags initiated" << std::endl;

		path_entropy.calculate_entropy_independent(1, flag_par, &entropy);
		std::cout << "entropy calculated" << std::endl;

		{
			std::string str = std::to_string(*(double *)entropy.total);
			std::cout << "entropy stringified" << std::endl;
			Text *t = new Text(str);
			t->setRight(0.2, 0.6);
			addTempObject(t);
		}
	}

	Scene::buttonPressed(tag, button);
}

void PathThermodynamics::refresh()
{
	Scene::refresh();
}
