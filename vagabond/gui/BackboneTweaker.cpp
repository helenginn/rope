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

#include "BackboneTweaker.h"
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/ThickLine.h>
#include <vagabond/core/ParamSet.h>
#include <vagabond/core/Parameter.h>
#include <vagabond/core/paths/Route.h>

BackboneTweaker::BackboneTweaker(Scene *scene, Motion &motion, Atom *const &atom,
                                 Parameter *const &param, Route *const &route) : 
PathTweaker(scene, motion, atom, param, route, 0.3)
{
	prepareParameters();
}

void BackboneTweaker::prepareParameters()
{
	auto filter_in = [this](Parameter *const &p)
	{
		return (p->coversMainChain() &&
		        _route->indexOfParameter(p) >= 0);
	};

	ParamSet set{_param};
	Parameter *last_left = _param;
	Parameter *last_right = _param;
	_idxs.clear();

	std::vector<Parameter *> lefts, rights;

	for (int i = 0; i < 5; i++)
	{
		ParamSet expanded{set};
		expanded.expandNeighbours(true);
		expanded.filter(filter_in);

		ParamSet remaining = expanded - set;
		set = expanded;
		
		if (remaining.size() == 2)
		{
			std::vector<Parameter *> ends = remaining.toVector();

			bool diff = ends.front()->residueId() != ends.back()->residueId();

			if (diff)
			{
				diff = ends.front()->residueId() < ends.back()->residueId();

				if (diff)
				{
					last_left = ends.front();
					last_right = ends.back();
				}
				else
				{
					last_left = ends.back();
					last_right = ends.front();
				}
			}
			else if (ends.front()->atomCount() == 4)
			{
				if (ends.front()->atomIsCentral(last_left->atom(0)))
				{
					last_left = ends.front();
					last_right = ends.back();
				}
				else if (ends.front()->atomIsCentral(last_right->atom(3)))
				{
					last_right = ends.front();
					last_left = ends.back();
				}
			}
			
			lefts.push_back(last_left);
			rights.push_back(last_right);
		}
	}
	
	for (auto it = lefts.rbegin(); it != lefts.rend(); it++)
	{
		int idx = _route->indexOfParameter(*it);
		_idxs.push_back(idx);
	}

	int idx = _route->indexOfParameter(_param);
	_idxs.push_back(idx);

	for (auto it = rights.begin(); it != rights.end(); it++)
	{
		int idx = _route->indexOfParameter(*it);
		_idxs.push_back(idx);
	}
}

void BackboneTweaker::redraw()
{
	Box *box = new Box();
	Box *blobs = new Box();
	Box *texts = new Box();
	float top = 0;
	float step = 0.6 / (float)(_idxs.size() + 2);
	ThickLine *line = new ThickLine(false);//, "assets/images/strong_bond.png");
	line->setThickness(0.1);
	line->reset();
	float cx = 0.8; float cy = 0.25;

	for (const int &idx : _idxs)
	{
		Parameter *const &p = _route->parameter(idx);
		Motion &motion = _route->motion(idx);
		float angle = motion.workingAngle();
		float left = angle * 0.15 / 360.f;

		ImageButton *ib = new ImageButton("assets/images/dot.png", this);
		ib->setCentre(left + cx, top + cy);
		ib->resize(0.02);
		ib->addAltTag(f_to_str(angle, 1) + " deg.", 0.0);
		
		auto flip_and_draw = [this, &motion]()
		{
			motion.flip = !motion.flip;
			redraw();
		};
		
		ib->setReturnJob(flip_and_draw);
		blobs->addObject(ib);
		
		line->addPoint({left + cx, top + cy, 0.9});

		std::string text = angle < 0 ?
		(p->residueId().str() + " " + p->desc()) : 
		(p->desc() + " " + p->residueId().str());
		
		Text *t = new Text(text);
		(angle < 0) ? t->setRight(left + cx + 0.01, top + cy) :
		t->setLeft(left + cx - 0.01, top + cy);
		t->resize(0.4);
		texts->addObject(t);
		
		top += step;
	}

	box->addObject(line);
	box->addObject(blobs);
	box->addObject(texts);

	deleteTemps();
	addTempObject(box);
}

void BackboneTweaker::setup()
{
	redraw();

}
