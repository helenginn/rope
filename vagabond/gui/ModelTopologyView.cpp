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

#include "ModelTopologyView.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/Chain.h>
#include <vagabond/core/PositionShifter.h>
#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/gui/elements/FloatingImage.h>

ModelTopologyView::ModelTopologyView(Scene *prev, 
                                     Model &contents)
: Scene(prev), Mouse3D(prev), _model(contents)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

	_model.load();

}

ModelTopologyView::~ModelTopologyView()
{
	_model.unload();
}

void ModelTopologyView::makeDots()
{
	AtomContent *ac = _model.currentAtoms();

	for (size_t i = 0; i < ac->chainCount(); i++)
	{
		Chain *ch = ac->chain(i);
		FloatingImage *fi
		= new FloatingImage("assets/images/circle.png", 10);
		fi->setPosition(ch->initialCentre());
		addObject(fi);
		glm::vec3 start = ch->initialCentre();
		_map[fi] = {ch, start};
		shiftToCentre(start, 0);
	}

}

void ModelTopologyView::makeShifter()
{
	_shifter = new PositionShifter(getModel());
	_shifter->run();
	
	auto make_init = [](ChainInfo &info)
	{
		glm::vec3 pos = info.pos;
		return [pos]()
		{
			return pos;
		};
	};
	
	auto make_getter = [](ChainInfo &info)
	{
		return [&info]()
		{
			return info.pos;
		};
	};

	auto make_setter = [](ChainInfo &info)
	{
		return [&info](const glm::vec3 &vec)
		{
			info.pos = vec;
		};
	};

	auto tidy = [this]()
	{
		for (auto it = _map.begin(); it != _map.end(); it++)
		{
			FloatingImage *fi = it->first;
			fi->setPosition(it->second.pos);
			fi->forceRender(true, false);
		}
	};
	
	_shifter->pause();
	for (auto it = _map.begin(); it != _map.end(); it++)
	{
		FloatingImage *fi = it->first;
		ChainInfo &info = it->second;
		_shifter->addPosition(fi, make_init(info),
		                      make_getter(info),
		                      make_setter(info));
	}
	
	_shifter->addTidy(tidy);
	_shifter->unpause();
}

void ModelTopologyView::addLinks()
{

}

void ModelTopologyView::setup()
{
	makeDots();
	_2D = true;
	makeShifter();
}
