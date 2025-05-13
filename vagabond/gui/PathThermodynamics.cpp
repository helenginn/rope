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
#include "CandidateView.h"
#include <vagabond/core/PathManager.h>
#include <vagabond/core/PathGroup.h>
#include <vagabond/core/Path.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/PathEntropy.h>
#include <nlohmann/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/BadChoice.h>

int pathNum = 0;

PathThermodynamics::PathThermodynamics(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths) : Scene(prev)
{
	_entity = entity;
	_paths = paths;
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
		
		if (pathNum < 1)
		{
			std::string str = "Choose number of paths (\"frames\") to utilise";
			ChooseRange *cr = new ChooseRange(this, str, "choose_paths", this);
			cr->setDefault(1);
			cr->setRange(1, _paths.size(), _paths.size()-1);
			setModal(cr);
	
			float num = cr->max();
			int pathNum = lrint(num);
		}
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

	refresh();
}

void PathThermodynamics::buttonPressed(std::string tag, Button *button)
{
	if (tag == "calc_indep")
	{
		struct Flag_par flag_par;
		struct Entropy entropy;
		
		Sequence *seq = _entity->sequence();

		PathEntropy path_entropy;

		std::cout << "Out of if statement..." << std::endl;
		const std::string mod_id = _paths[0].front()->startInstance()->model_id();

		Tors_res4nn* tors_res = new Tors_res4nn[seq->size()]{};

		path_entropy.init_flag_par(&flag_par);	
		path_entropy.get_atoms_and_residues(mod_id, *tors_res);

		path_entropy.calculate_entropy_independent(pathNum, flag_par, seq, tors_res, &entropy);
		std::cout << "entropy calculated" << std::endl;

		{
			std::string str = "Total: " +  std::to_string(*(double *)entropy.total) + " (R units)\n" + "Per residue: " + std::to_string(*(double *)entropy.total/seq->size()) + " (R units)";
			Text *t = new Text(str);
			t->setLeft(0.2, 0.6);
			addTempObject(t);
		}
	}

	Scene::buttonPressed(tag, button);
}

void PathThermodynamics::refresh()
{
	Scene::refresh();
}
