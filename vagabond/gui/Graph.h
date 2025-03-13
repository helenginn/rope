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

#ifndef __vagabond__Graph__
#define __vagabond__Graph__

#include <vagabond/gui/elements/Box.h>
#include <map>

class ThickLine;

class Graph : public Box
{
public:
	Graph();

	void addPoint(int series, float x, float y);
	void setRange(char axis, float min, float max);
	
	// width and height of box contents.
	void setup(float width, float height);
	void addToGraphPosition(float cx, float cy);
	
	void setSeriesColour(int series, glm::vec3 colour);
	
	void setAxisLabel(char axis, std::string name);
private:
	void addAxes(float width, float height);
	void addAxisTicks(int axis, float width, float height);
	void addAxisLabels(int axis, float width, float height);
	void addLines(float width, float height);
	void addLine(float width, float height, int series, 
	             std::vector<glm::vec2> &line);
	void determineLimits();
	void loadLine(ThickLine *tl, glm::vec3 &start, const glm::vec3 &move,
	              glm::vec3 colour = {0.2, 0.2, 0.2});

	std::map<int, std::vector<glm::vec2>> _data;
	std::map<int, glm::vec2> _axisRanges;
	
	std::map<int, std::string> _labels;
	std::map<int, glm::vec3> _colours;
};

#endif
