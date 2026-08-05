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
#include "Scatter.h"
//#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/ThickLine.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/elements/Text.h>
#include <sstream>
#include <iomanip>

Graph::Graph()
{
	clearVertices();

}

void Graph::addPoint(int series, float x, float y, const std::string &label,
                     float alpha, int pointType)
{
	_data[series].push_back({{x, y}, label, alpha, pointType});
}

void Graph::setRange(char axis, float min, float max)
{
	int num = (axis == 'x' ? 0 : 1);
	_axisRanges[num] = {min, max};
}

void Graph::loadLine(ThickLine *tl, glm::vec3 &start, const glm::vec3 &move,
                     glm::vec3 colour)
{
	int divs = lrint(glm::length(move) / 0.05);
	if (divs == 0) divs = 1;
	float ratio = glm::length(move) / 0.05f;

	glm::vec3 dir = move / (float)divs;

	for (int i = 0; i <= divs; i++)
	{
		glm::vec3 point = start;
		tl->addPoint(point, colour);
		start += dir;
		
		// last vertices: to be scaled by ratio if ratio < 1.
		if (ratio > 1 || tl->vertexCount() < 4)
		{
			continue;
		}

		for (int j = tl->vertexCount() - 4; j < tl->vertexCount(); j++)
		{
			Vertex v = tl->vertex(j);
			v.tex[1] *= ratio;
			v.normal /= ratio;
			tl->setVertex(j, v);
		}
		
	}
}

void Graph::addAxes(float width, float height)
{
	glm::vec3 start = {-width / 2.f, +height / 2.f, 0};
	glm::vec3 move_x = {width, 0.f, 0.f};
	glm::vec3 move_y = {0.f, -height, 0.f};

	glm::vec3 xstart = start;
	ThickLine *tlx = new ThickLine(false, "assets/images/vertical_line.png");
	loadLine(tlx, xstart, move_x);
	tlx->setThickness(0.5 * Window::aspect());
	addObject(tlx);

	glm::vec3 ystart = start;
	ThickLine *tly = new ThickLine(false, "assets/images/vertical_line.png");
	loadLine(tly, ystart, move_y);
	tly->setThickness(0.5 * Window::aspect());
	addObject(tly);
}

void Graph::addAxisTicks(int axis, float width, float height)
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

	float log_min = log(_axisRanges[axis][0]) / log(best_log);
	float mod_min_bx = floor(log_min);
	float min = pow(best_log, mod_min_bx);
	
	float divide = 5;
	int best_logi = (long int)lrint(best_log);
	switch (best_logi)
	{
		case 2: divide = 4; break;
		case 5: divide = 5; break;
		case 10: divide = 5; break;
		default: break;
	}

	float one_tick = total / divide;
	std::cout << total << " / " << divide << std::endl;

	float xmult = (axis == 0 ? 1 : 0);
	float ymult = (axis == 1 ? 1 : 0);
	glm::vec2 start = {-width / 2.f, +height / 2.f};
	glm::vec2 offset = {-width * 0.04f * ymult, +height * 0.06f * xmult};
	glm::vec2 stride = {width, -height};
	float tick_step = one_tick / diff;
	divide = floor(diff / one_tick);
	Renderable::Alignment a{};
	a = axis == 0 ?  Renderable::Centre : Renderable::Right;
	
	float min_tick = std::round(_axisRanges[axis][0] / one_tick) * one_tick;

	for (int i = 0; i <= divide; i++)
	{
		float val = one_tick * i + min_tick;
		float prop = (val - _axisRanges[axis][0]) / diff;
		if (prop > 1 || prop < 0)
		{
			continue;
		}

		std::ostringstream ss;
		if (val > 1)
		{
			ss << std::defaultfloat << val;
		}
		else
		{
			ss << std::setprecision(3) << val;
		}
		
		int precision = val < 1 ? 3 : 1;
		Text *text = new Text(ss.str());
		glm::vec2 xy = {(stride * prop).x * xmult, 
		                (stride * prop).y * ymult};
		xy += start + offset;
		text->resize(0.8);
		text->setArbitrary(xy.x, xy.y, a);
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
		tick->setArbitrary(xy.x - offset.x, xy.y - offset.y, a);
		addObject(tick);
	}
}

void Graph::addAxisLabels(int axis, float width, float height)
{
	if (_labels.count(axis) == 0)
	{
		return;
	}

	std::string &label = _labels[axis];

	float xmult = (axis == 0 ? 1 : 0);
	float ymult = (axis == 1 ? 1 : 0);

	glm::vec2 start = {-width * 0.5f, +height * 0.7f};
	glm::vec2 stride = {width * 0.5f * xmult, -height * 1.3f * ymult};
	
	glm::vec2 pos = start + stride;

	Text *text = new Text(label);
	text->setCentre(pos.x, pos.y);
	addObject(text);
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
			const std::vector<DataPoint> &vals = it->second;
			
			for (const DataPoint &dp : vals)
			{
				if (dp.point[i] < min) min = dp.point[i];
				if (dp.point[i] > max) max = dp.point[i];
			}
		}
		
		_axisRanges[i] = {min, max};
	}
}

void Graph::addLine(float width, float height, int series, 
                    std::vector<DataPoint> &line)
{
	glm::vec2 start = {-width / 2.f, +height / 2.f};
	glm::vec2 stride = {width, -height};
	const float &xmin  = _axisRanges[0][0];
	const float &xmax  = _axisRanges[0][1];
	const float &ymin  = _axisRanges[1][0];
	const float &ymax  = _axisRanges[1][1];

	glm::vec2 diff = {xmax - xmin, ymax - ymin};

	ThickLine *tl = new ThickLine(false, "assets/images/vertical_line.png");
	
	bool first = true;
	glm::vec3 last{};
	for (const DataPoint &dp : line)
	{
		const glm::vec2 &point = dp.point;
		glm::vec2 scaled = glm::vec2((point.x - xmin) / diff.x, 
		                             (point.y - ymin) / diff.y);
		glm::vec2 offset = glm::vec2(stride.x * scaled.x, stride.y * scaled.y);
		glm::vec3 pos = glm::vec3(start + offset, 0.f);
		if (first)
		{
			first = false;
		}
		else
		{
			glm::vec3 c = {0.2f, 0.2f, 0.2f};
			if (_colours.count(series))
			{
				c = _colours[series];
			}
			loadLine(tl, last, (pos - last), c);
		}

		last = pos;
	}

	tl->setThickness(0.1);
	addObject(tl);
}

void Graph::plotData(float width, float height)
{
	if (style == StyleLine)
	{
		addLines(width, height);
	}
	else if (style == StyleScatter)
	{
		addScatters(width, height);
	}
}

void Graph::addLines(float width, float height)
{
	for (auto it = _data.begin(); it != _data.end(); it++)
	{
		std::vector<DataPoint> &line = it->second;
		addLine(width, height, it->first, line);
	}
}

void Graph::addPoints(float width, float height, int series,
                     std::vector<DataPoint> &line)
{
	glm::vec2 start = {-width / 2.f, +height / 2.f};
	glm::vec2 stride = {width, -height};
	const float &xmin  = _axisRanges[0][0];
	const float &xmax  = _axisRanges[0][1];
	const float &ymin  = _axisRanges[1][0];
	const float &ymax  = _axisRanges[1][1];

	glm::vec2 diff = {xmax - xmin, ymax - ymin};
	Scatter *sc = new Scatter(this, _scatters.size());
	_scatters.push_back(sc);

	for (const DataPoint &dp : line)
	{
		const glm::vec2 &point = dp.point;
		glm::vec2 scaled = glm::vec2((point.x - xmin) / diff.x, 
		                             (point.y - ymin) / diff.y);
		glm::vec2 offset = glm::vec2(stride.x * scaled.x, stride.y * scaled.y);
		glm::vec3 pos = glm::vec3(start + offset, 0.f);
		glm::vec3 c = {0.2f, 0.2f, 0.2f};
		if (_colours.count(series))
		{
			c = _colours[series];
		}

		sc->addPoint(pos, c, dp.pointType, dp.alpha);
	}

	addObject(sc);
}

void Graph::addScatters(float width, float height)
{
	for (auto it = _data.begin(); it != _data.end(); it++)
	{
		std::vector<DataPoint> &line = it->second;
		addPoints(width, height, it->first, line);
	}
}

void Graph::clear()
{
	clearObjects();
	_scatters.clear();
	_missesSinceShown = 0;
}

void Graph::setup(float width, float height)
{
	clear();
	determineLimits();
	addAxes(width, height);
	addAxisTicks(0, width, height);
	addAxisTicks(1, width, height);
	addAxisLabels(0, width, height);
	addAxisLabels(1, width, height);
	plotData(width, height);
}

void Graph::addToGraphPosition(float cx, float cy)
{
	glm::vec3 unit = glm::vec3(cx * 2, -cy * 2, 0);
	addToVertices(unit);

}

void Graph::setAxisLabel(char axis, std::string name)
{
	int num = (axis == 'x' ? 0 : 1);
	_labels[num] = name;
}

void Graph::setSeriesColour(int series, glm::vec3 colour)
{
	_colours[series] = colour;
}

void Graph::setIndexResponder(IndexResponseView *irv)
{
	for (Scatter *sc : _scatters)
	{
		irv->addIndexResponder(sc);
	}

}

void Graph::noteLabelShown()
{
	_missesSinceShown = 0;
}

void Graph::clearLabels()
{
	// IndexResponseView::checkIndexBuffer() picks per-pixel, straight off
	// the point's own rendered (antialiased, discard-edged - see
	// point.fsh) footprint, with no positional tolerance at all - so
	// ordinary mouse jitter right at a point's visible edge oscillates
	// between "over the point" (Scatter::interacted(), draws the label)
	// and "over nothing" (this) on essentially every mouse-move poll,
	// which without this debounce wiped and immediately redrew the label
	// every single time, i.e. visible flicker. Only actually clearing
	// once a few misses have happened in a row - reset the moment
	// interacted() fires again via noteLabelShown() - absorbs that edge
	// jitter (which never strings together this many consecutive misses
	// without an intervening hit) while still clearing promptly once the
	// mouse has genuinely moved away from every point.
	_missesSinceShown++;
	if (_missesSinceShown < 3)
	{
		return;
	}

	for (Scatter *sc : _scatters)
	{
		sc->deleteTemps();
	}
}
