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

#include "ShowMap.h"
#include "VagWindow.h"
#include "MapView.h"
#include <vagabond/core/SpecificNetwork.h>
#include <vagabond/core/Instance.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/core/Network.h>
#include <vagabond/core/Cartographer.h>
#include <vagabond/utils/Mapping.h>

ShowMap::ShowMap(Scene *scene, SpecificNetwork *sn) : Scene(scene)
{
	_sn = sn;
	_line = new Line(false);
	addObject(_line);
	update(nullptr);
}

void ShowMap::setup()
{
	addTitle("Map vertices");
}

void ShowMap::updateFromMap(Mapped<float> *map)
{
	_line->clearVertices();

	if (!map)
	{
		return;
	}

	std::vector<float> min, max;
	map->bounds(min, max);
	
	glm::mat3x3 inv = glm::mat3(1.f);
	for (int i = 0; i < 2; i++)
	{
		inv[i][i] = (max[i] - min[i]) * 2;
	}
	glm::mat3x3 model = glm::inverse(inv);
	model[0][0] *= Window::aspect();
	Network *net = _sn->network();
	
	Box *box = new Box();
	for (size_t i = 0; i < map->pointCount(); i++)
	{
		std::vector<float> vec = map->point_vector(i);
		glm::vec3 empty{};
		
		for (size_t j = 0; j < vec.size() && j < 2; j++)
		{
			empty[j] = vec[j];
		}
		
		glm::vec3 pos = model * empty;
		pos *= 0.8;
		
		std::string id = net->instances()[i]->id();

		ImageButton *image = new ImageButton("assets/images/orange.png", this);
		image->setReturnTag("select_" + std::to_string(i));
		image->setCentre(pos.x, -pos.y);
		image->resize(0.02);
		image->addAltTag(id);
		box->addObject(image);
		
		_line->addPoint(image->centroid());
	}
	
	box->setCentre(0.5, 0.5);
	addObject(box);
	
	for (size_t i = 0; i < map->faceCount(); i++)
	{
		std::vector<int> pIndices;
		map->point_indices_for_face(i, pIndices);

		for (size_t j = 1; j < pIndices.size(); j++)
		{
			_line->addIndex(pIndices[j - 1]);
			_line->addIndex(pIndices[j]);
		}

		_line->addIndex(pIndices.front());
		_line->addIndex(pIndices.back());
	}
	
	_line->setCentre(0.5, 0.5);
	_line->forceRender(true, true);
}

void ShowMap::update(Parameter *param)
{
	Mapped<float> *map = nullptr;
	if (!param)
	{
		map = _sn->network()->blueprint();
	}
	else
	{
		map = _sn->mapForParameter(param);
	}

	std::cout << map << std::endl;
	_parameter = param;
	updateFromMap(map);
}

void ShowMap::buttonPressed(std::string tag, Button *button)
{
	std::string select = Button::tagEnd(tag, "select_");
	
	if (select.length())
	{
		Menu *menu = new Menu(this, this, "option");
		menu->addOption("flip torsions", "flip_" + select);
		menu->addOption("set as reference", "ref_" + select);
		menu->setReturnObject(button);
		menu->setup(button);
		setModal(menu);
	}

	std::string id = Button::tagEnd(tag, "option_flip_");
	if (id.length())
	{
		int idx = atoi(id.c_str());
		_first = idx;
		
		if (_second < 0)
		{
			setInformation("Choose connected structure to match torsions with");
		}
	}

	id = Button::tagEnd(tag, "option_ref_");
	if (id.length())
	{
		int idx = atoi(id.c_str());
		_second = idx;
		
		if (_first < 0)
		{
			setInformation("Now flip torsions of a connected structure");
		}
	}
	
	if (_first >= 0 && _second >= 0 && _first != _second)
	{
		_view->startFlip(_second, _first);
		back();
	}

	Scene::buttonPressed(tag, button);
}
