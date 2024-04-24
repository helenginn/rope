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

#include "ChooseHeader.h"
#include "WholeModelPathSetupView.h"
#include "PathInterfaceSetupView.h"
#include "ListInstancesView.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/ModelManager.h>

WholeModelPathSetupView::WholeModelPathSetupView(Scene *prev) : Scene(prev)
{

}

void WholeModelPathSetupView::setup()
{
	addTitle("Whole model path setup");
	float top = 0.3;

	{
		Text *t = new Text("Choose \"from\" model");
		t->setLeft(0.2, top);
		t->addAltTag("Path will fetch entities from this starting model\n");
		addObject(t);
	}
	{
		TextButton *t = new TextButton("choose...", this);
		t->setRight(0.8, top);
		t->setReturnJob([this]() { getStructure(true); });
		_fromButton = t;
		addObject(t);
	}

	top += 0.06;

	{
		Text *t = new Text("Choose \"to\" model");
		t->setLeft(0.2, top);
		t->addAltTag("Path end point will match instances from "\
		             "this model.");
		addObject(t);
	}
	{
		TextButton *t = new TextButton("choose...", this);
		t->setRight(0.8, top);
		t->setReturnJob([this]() { getStructure(false); });
		_toButton = t;
		addObject(t);
	}
	
	top += 0.06;
}

std::vector<std::string> instances_for(std::string &id)
{
	std::vector<std::string> list;
	Model *model = ModelManager::manager()->model(id);
	if (model)
	{
		std::vector<Instance *> instances = model->instances();
		for (Instance *inst : instances)
		{
			if (inst->isRefined())
			{
				list.push_back(inst->id());
			}
		}
	}

	return list;
}

void WholeModelPathSetupView::refresh()
{
	_fromButton->setText(_fromId == "" ? "choose..." : _fromId);
	_toButton->setText(_toId == "" ? "choose..." : _toId);

	deleteTemps();

	if (_fromId.length() && _toId.length())
	{
		int chosen = chosenPairs();
		{
			TextButton *t = new TextButton("Choose included instances", this);
			t->setLeft(0.2, 0.45);
			t->addAltTag("Sort through proteins/ligands/waters and choose \n"
			             "those to include");
			t->setReturnJob
			([this]()
			 {
				std::vector<std::string> froms = instances_for(_fromId);
				std::vector<std::string> tos = instances_for(_toId);

				ListInstancesView *view = new ListInstancesView(this);
				view->setResponder(this);
				view->setMap(_map);
				view->setFromList(froms);
				view->setToList(tos);
				view->show();

			 });
			addTempObject(t);
			
			std::string str = std::to_string(chosen) + " pair";
			str += chosen == 1 ? "" : "s";
			Text *info = new Text(str);
			info->setRight(0.8, 0.45);
			addTempObject(info);
		}

		if (chosen > 0)
		{
			TextButton *t = new TextButton("Next", this);
			t->setRight(0.8, 0.8);
			t->setReturnJob
			([this]()
			 {
				std::map<std::string, std::string> simple_map;
				for (auto it = _map.begin(); it != _map.end(); it++)
				{
					auto &pair = it->second;
					if (pair.active && pair.id.length() > 0)
					{
						simple_map[it->first] = pair.id;
					}
				}
				
				Model *from = ModelManager::manager()->model(_fromId);
				Model *to = ModelManager::manager()->model(_toId);

				PathInterfaceSetupView *view;
				view = new PathInterfaceSetupView(this, from, to);
				view->setMap(simple_map);
				view->show();
			 });

			addTempObject(t);
		}
	}

	Scene::refresh();
}

void WholeModelPathSetupView::getStructure(bool from)
{
	_from = from;

	std::list<Model> &models = ModelManager::manager()->objects();
	std::set<std::string> ids;

	for (Model &model : models)
	{
		ids.insert(model.id());
	}
	
	ChooseHeader *ch = new ChooseHeader(this, true);
	ch->setResponder(this);
	ch->setHeaders(ids);
	std::string descriptor = (from ? "\"from\"" : "\"to\"");
	ch->setTitle("Set " + descriptor + " model");
	ch->show();
}

void WholeModelPathSetupView::sendObject(std::string tag, void *object)
{
	if (tag == "instance_map" && object)
	{
		_map = *static_cast<ListInstancesView::Map *>(object);
		
		for (auto it = _map.begin(); it != _map.end(); it++)
		{
			auto &pair = it->second;
			if (pair.active && pair.id.length() == 0)
			{
				pair.active = false;
			}
			if (!pair.active && pair.id.length())
			{
				pair.id = "";
			}
		}
		refresh();
	}
	else
	{
		std::string &chosen = (_from ? _fromId : _toId);
		chosen = tag;
		_map = {};
	}
}

size_t WholeModelPathSetupView::chosenPairs()
{
	size_t count = 0;
	for (auto it = _map.begin(); it != _map.end(); it++)
	{
		auto &pair = it->second;
		if (pair.active && pair.id.length() > 0)
		{
			count++;
		}
	}

	return count;
}
