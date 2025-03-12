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

#include "Graph.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/ThickLine.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/elements/Text.h>

Graph::Graph()
{
	clearVertices();

}

void Graph::addDataPoint(int series, float x, float y)
{
	_data[series].push_back({x, y});
}

void Graph::setRange(char axis, float min, float max)
{
	int num = (axis == 'x' ? 0 : 1);
	_axisRanges[num] = {min, max};
}

void Graph::addAxes(float width, float height)
{
	auto load_thick_line = [this](ThickLine *tl, glm::vec3 &start,
	                          const glm::vec3 &move)
	{
		int divs = (glm::length(move) / 0.05) + 1;
		glm::vec3 dir = move / (float)divs;

		for (int i = 0; i <= divs; i++)
		{
			glm::vec3 point = start;
			tl->addPoint(point, {0.2, 0.2, 0.2});
			start += dir;
		}
		
		tl->setThickness(0.5 * Window::aspect());
		addObject(tl);
	};
	
	glm::vec3 start = {-width / 2.f, +height / 2.f, 0};
	glm::vec3 move_x = {width, 0.f, 0.f};
	glm::vec3 move_y = {0.f, -height, 0.f};

	glm::vec3 xstart = start;
	ThickLine *tlx = new ThickLine(false, "assets/images/vertical_line.png");
	load_thick_line(tlx, xstart, move_x);

	glm::vec3 ystart = start;
	ThickLine *tly = new ThickLine(false, "assets/images/vertical_line.png");
	load_thick_line(tly, ystart, move_y);
}

void Graph::addAxisLabels(int axis, float width, float height)
{
	float diff = _axisRanges[axis][1] - _axisRanges[axis][0];
	float log_diff_b10 = log(diff) / log(10.f);
	float log_diff_b5 = log(diff) / log(5.f);
	float log_diff_b2 = log(diff) / log(2.f);

	float mod_b10 = floor(log_diff_b10);
	float mod_b5 = floor(log_diff_b5);
	float mod_b2 = floor(log_diff_b2);
	
	float diff_b10 = log_diff_b10 - mod_b10;
	float diff_b5 = log_diff_b5 - mod_b5;
	float diff_b2 = log_diff_b2 - mod_b2;
	
	float best_log = 10.f;
	if (diff_b5 < diff_b10 && diff_b5 < diff_b2) best_log = 5.f;
	if (diff_b2 < diff_b5 && diff_b2 < diff_b10) best_log = 2.f;
	
	float log_diff = log(diff) / log(best_log);
	float mod_bx = floor(log_diff);
	float total = pow(best_log, mod_bx);
	float one_tick = total / 5;

	/*
	std::cout << "diff: " << diff << std::endl;
	std::cout << "log_diff_b10: " << log_diff_b10 << std::endl;
	std::cout << "log_diff_b5: " << log_diff_b5 << std::endl;
	std::cout << "log_diff_b2: " << log_diff_b2 << std::endl;
	std::cout << "best_log: " << best_log << std::endl;
	std::cout << "log_diff: " << log_diff << std::endl;
	std::cout << "mod_bx: " << mod_bx << std::endl;
	std::cout << "total: " << total << std::endl;
	std::cout << "tick: " << one_tick << std::endl;
	std::cout << std::endl;
	*/

	float xmult = (axis == 0 ? 1 : 0);
	float ymult = (axis == 1 ? 1 : 0);
	glm::vec2 start = {-width / 2.f, +height / 2.f};
	glm::vec2 offset = {-width * 0.1f * ymult, +height * 0.1f * xmult};
	glm::vec2 stride = {width, -height};
	float tick_step = one_tick / diff;
	std::cout << "tick step " << tick_step << std::endl;
	stride *= tick_step;

	for (int i = 0; i <= 5; i++)
	{
		float val = one_tick * i;
		int precision = val < 1 ? 3 : 1;
		Text *text = new Text(f_to_str(val, precision));
		glm::vec2 xy = {(stride * (float)i).x * xmult, 
		                (stride * (float)i).y * ymult};
		xy += start + offset;
		std::cout << "i: " << i << " " << xy << std::endl;
		text->setCentre(xy.x, xy.y);
		addObject(text);
		
		Image *tick = new Image("assets/images/little_line.png");

		if (axis == 0)
		{
			glm::mat3x3 rot;
			rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(90.f),
			                              glm::vec3(0., 0., -1.)));
			tick->rotateRoundCentre(rot);
		}

		tick->resize(0.02);
		Renderable::Alignment a{};
		a = axis == 0 ? 
		    Renderable::Alignment(Renderable::Centre | Renderable::Top) :
		    Renderable::Alignment(Renderable::Right | Renderable::Middle);
		tick->setArbitrary(xy.x - offset.x, xy.y - offset.y, a);
		addObject(tick);
	}
	std::cout << std::endl;
}

void Graph::determineLimits()
{
	for (size_t i = 0; i < 2; i++)
	{
		if (_axisRanges.count(i))
		{
			if (_axisRanges[i][1] < _axisRanges[i][0])
			{
				float tmp = _axisRanges[i][1];
				_axisRanges[i][1] = _axisRanges[i][0];
				_axisRanges[i][0] = tmp;
			}

			continue;
		}

		float min = FLT_MAX;
		float max = -FLT_MAX;

		for (auto it = _data.begin(); it != _data.end(); it++)
		{
			const std::vector<glm::vec2> &vals = it->second;
			
			for (const glm::vec2 &val : vals)
			{
				if (val[i] < min) min = val[i];
				if (val[i] > max) max = val[i];
			}
		}
		
		_axisRanges[i] = {min, max};
	}
}

void Graph::setup(float width, float height)
{
	determineLimits();
	addAxes(width, height);
	_axisRanges[0] = {0, 106};
	_axisRanges[1] = {0, 0.24};
	addAxisLabels(0, width, height);
	addAxisLabels(1, width, height);
}
