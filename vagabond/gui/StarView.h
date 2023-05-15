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

#ifndef __vagabond__StarView__
#define __vagabond__StarView__

#include "PointyView.h"

class TorsionCluster;
class Path;
class Line;

class StarView : public PointyView
{
public:
	StarView();
	
	void setCluster(TorsionCluster *cluster);
	
	void setSteps(int steps)
	{
		_steps = steps;
	}
	
	void setPaths(std::set<Path *> &paths);

	virtual void updatePoints();
	virtual void makePoints();
private:
	void colourVertex(Vertex &v);
	void incrementColour();

	TorsionCluster *_cluster = nullptr;
	Line *_line = nullptr;

	int _steps = 12;
	float hue = 0;
	
	std::set<Path *> _paths;
};

#endif
