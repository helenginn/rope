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
#include <vagabond/core/GroupBounds.h>
#include <vagabond/core/PositionShifter.h>
#include <vagabond/gui/elements/Line.h>
#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/gui/TopologyCircle.h>
#include <vagabond/gui/elements/FloatingText.h>

ModelTopologyView::ModelTopologyView(Scene *prev, 
                                     Model &contents)
: Scene(prev), Mouse3D(prev), IndexResponseView(prev), _model(contents)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

	_model.load();

}

ModelTopologyView::~ModelTopologyView()
{
	delete _shifter;
	_model.unload();
}

void ModelTopologyView::makeDots()
{
	AtomContent *ac = _model.currentAtoms();

	for (size_t i = 0; i < ac->chainCount(); i++)
	{
		Chain *ch = ac->chain(i);
		TopologyCircle *fi = new TopologyCircle(this);
		FloatingText *ft = new FloatingText(ch->id(), 100);

		glm::vec3 start = ch->initialCentre();
		fi->FloatingImage::setPosition(start);
		ft->FloatingText::setPosition(start);

		_image2Info[fi] = {ch, start, ft};
		_chain2Image[ch] = fi;
		shiftToCentre(start, 0);
	}

}

void ModelTopologyView::makeShifter()
{
	_shifter = new PositionShifter(getModel());
//	_shifter->setUniformDistance(10.f);
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
		for (auto it = _image2Info.begin(); 
		     it != _image2Info.end(); it++)
		{
			TopologyCircle *fi = it->first;
			fi->FloatingImage::setPosition(it->second.pos);
			fi->FloatingImage::forceRender(true, false);

			FloatingText *ft = it->second.text;
			ft->setPosition(it->second.pos);
			ft->forceRender(true, false);
		}
	};
	
	_shifter->pause();
	for (auto it = _image2Info.begin(); it != _image2Info.end(); it++)
	{
		TopologyCircle *fi = it->first;
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
	float max = 8.f;
	auto valid_distance = [max](glm::vec3 &a, glm::vec3 &b) -> bool
	{
		bool bad = false;
		for (int i = 0; i < 3; i++)
		{
			if (fabs(a[i] - b[i]) > max)
			{
				return false;
			}
		}

		if (glm::dot(a - b, a - b) > max * max)
		{
			return false;
		}

		return true;
	};

	auto close_contact = [valid_distance](Chain *chain, Chain *other)
	{
		for (Atom *atom : chain->atomVector())
		{
			glm::vec3 p1 = atom->initialPosition();
			for (Atom *other : other->atomVector())
			{
				glm::vec3 p2 = other->initialPosition();
				if (valid_distance(p1, p2))
				{
					return true;
				}
			}
		}

		return false;
	};

	AtomContent *ac = _model.currentAtoms();

	_shifter->pause();
	for (size_t i = 0; i < ac->chainCount() - 1; i++)
	{
		Chain *left = ac->chain(i);
		GroupBounds bounds(left);

		OpSet<void *> list;
		for (size_t j = i + 1; j < ac->chainCount(); j++)
		{
			Chain *right = ac->chain(j);
			if (!bounds.worth_checking_interface_with(right, max + 2))
			{
				continue;
			}

			if (!close_contact(left, right))
			{
				continue;
			}
			
			Line *line = new Line();
			line->addPoint(posForChain(left));
			line->addPoint(posForChain(right));
			addObject(line);
			_links[line] = {_chain2Image[left], _chain2Image[right]};
			list += (void *)_chain2Image[right];
		}
		_shifter->limitSensitivity(_chain2Image[left], list);
	}
	
	auto tidy = [this]()
	{
		for (auto it = _links.begin(); it != _links.end(); it++)
		{
			Line *const &l = it->first;
			l->vertices()[0].pos = _image2Info[it->second.left].pos;
			l->vertices()[1].pos = _image2Info[it->second.right].pos;
			l->forceRender(true, false);
		}
	};

	_shifter->addTidy(tidy);
	_shifter->unpause();
}

void ModelTopologyView::addCircles()
{
	for (auto it = _image2Info.begin(); it != _image2Info.end(); it++)
	{
		TopologyCircle *fi = it->first;
		addObject((FloatingImage *)fi);
		addIndexResponder(fi);
		addObject(it->second.text);
	}
}

void ModelTopologyView::setup()
{
	makeDots();
	_2D = true;
	makeShifter();
	addLinks();
	addCircles();
	updateColours();
	IndexResponseView::setup();
}

void ModelTopologyView::unhighlight()
{
	for (auto it = _image2Info.begin(); it != _image2Info.end(); it++)
	{
		TopologyCircle *fi = it->first;
		fi->FloatingImage::setHighlighted(false);
	}

}

void ModelTopologyView::clicked(TopologyCircle *circle)
{
	if (_clickChainEvent)
	{
		ChainInfo &info = _image2Info[circle];
		float x, y;
		getFractionalPos(x, y);
		_clickChainEvent(this, info.ch, {x, y, 0});
	}
}

void ModelTopologyView::updateColours()
{
	if (!_colourFunc)
	{
		return;
	}

	for (auto it = _image2Info.begin(); it != _image2Info.end(); it++)
	{
		TopologyCircle *fi = it->first;
		Chain *ch = it->second.ch;
		glm::vec3 colour = _colourFunc(ch);
		fi->FloatingImage::setColour(colour.x, colour.y, colour.z);
		fi->FloatingImage::forceRender(true, false);
	}
}

void ModelTopologyView::interactedWithNothing(bool left, bool hover)
{
	unhighlight();
}

