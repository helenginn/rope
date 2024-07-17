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

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ThickLine.h>
#include <vagabond/gui/elements/Line.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Parameter.h>
#include <vagabond/core/paths/RTMotion.h>
#include <vagabond/core/paths/Route.h>
#include <vagabond/core/WayPoint.h>
#include "ParamTweaker.h"

ParamTweaker::ParamTweaker(Scene *scene, Motion &motion, Atom *const &atom,
                           Parameter *const &param, Route *const &route) : 
PathTweaker(scene, motion, atom, param, route, 0.2)
{

}

void ParamTweaker::getTrajectory()
{
	if (!_line)
	{
		_line = new ThickLine();
		addObject(_line);
	}

	_line->reset();
	_route->setDisplayAnything(false);

	for (float f = 0.0; f <= 1.01; f += 0.05)
	{
		_route->submitToShow(f, _atom);
		_route->retrieve();
		_line->addPoint(_atom->otherPosition("other"));
	}
	_route->setDisplayAnything(true);

	_line->forceRender();

	redraw();
}

void ParamTweaker::redraw()
{
	if (!_param->coversMainChain())
	{
		_route->submitToShow();
		_route->retrieve();
	}
}

void ParamTweaker::setup()
{
	getTrajectory();

	Text *t = new Text(_param->desc());
	t->resize(0.6);
	t->setCentre(0.85, 0.3);
	addObject(t);

	Text *t2 = new Text("tmp");
	t2->resize(0.4);
	t2->setCentre(0.85, 0.34);
	addObject(t2);
	
	ImageButton *b = new ImageButton("assets/images/refresh.png", this);
	b->resize(0.05);
	b->setCentre(0.82, 0.4);
	addObject(b);
	
	TextButton *tb = new TextButton("tmp", this);
	tb->resize(0.6);
	tb->setCentre(0.87, 0.4);
	addObject(tb);
	
	auto set_flip_name = [this, tb, t2]()
	{
		std::string val = f_to_str(_motion.workingAngle(), 1);
		t2->setText("(" + val + " degrees)");

		std::string flip = _motion.flip ? "yes" : "no";
		tb->setText(flip);
	};
	
	set_flip_name();

	auto flip_motion = [this, set_flip_name]()
	{
		_motion.flip = !_motion.flip;
		set_flip_name();
		getTrajectory();
	};
	
	b->setReturnJob(flip_motion);
	tb->setReturnJob(flip_motion);

	{
		TextButton *tb = new TextButton("ZERO", this, false, Font::Thick);
		tb->setColour(0.2, 0.2, 0.2);
		tb->resize(0.6);
		tb->setCentre(0.85, 0.5);
		addObject(tb);
		
		auto zero_motion = [this]()
		{
			_motion.wp.zero();
			getTrajectory();
			refreshHarmonics();
		};

		tb->setReturnJob(zero_motion);
	}
	
	setupHarmonics();
}

#define X_OFFSET 0.775
#define Y_OFFSET 0.7
#define Y_SCALE 720.f

glm::vec3 transform_to_pos(float frac, glm::vec3 start, float dist, 
                      float magnitude, float order)
{
	float amp = WayPoints::amplitude(frac, order);
	amp *= magnitude;
	amp /= Y_SCALE;
	float prop = frac * dist;

	return start + glm::vec3(prop, amp, 0.f);
}

void ParamTweaker::refreshHarmonics()
{
	for (int i = 0; i < _route->order(); i++)
	{
		refreshHarmonics(i);
	}
}

void ParamTweaker::refreshHarmonics(int i)
{
	_harmonics[i]->reset();

	for (float f = 0.0; f <= 1.01; f += 0.05)
	{
		float mag = _motion.wp._amps[i];
		glm::vec3 pos = transform_to_pos(f, {X_OFFSET, Y_OFFSET, 0.95},
		                                 0.15, mag, i);
		_harmonics[i]->addPoint(pos, glm::vec3(0, 0, 0));
	}

	_harmonics[i]->forceRender();
}

void ParamTweaker::setupHarmonics()
{
	Text *t = new Text("Harmonic\nperturbations");
	t->resize(0.8);
	t->setLeft(0.8, 0.6);
	addObject(t);

	for (int i = 0; i < _route->order(); i++)
	{
		_harmonics.push_back(new ThickLine(false));
		ThickLine *line = _harmonics.back();
		line->setDraggable(true);
		line->setSelectable(true);
		line->reset();

		auto alter_amp = [this, i](double x, double y)
		{
			y += (Y_OFFSET * 2 - 1);
			y = -y;
			_motion.wp._amps[i] = y * Y_SCALE / 2;
			refreshHarmonics(i);
			getTrajectory();
		};
		
		line->setJob(alter_amp);
		refreshHarmonics(i);
	}
		
	for (int i = _route->order() - 1; i >= 0; i--)
	{
		addObject(_harmonics[i]);
	}

}
