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

#ifndef __vagabond__ThickLine__
#define __vagabond__ThickLine__

#include "Image.h"

class ThickLine : public Image
{
public:
	ThickLine(bool proj = true);

	void addPoint(glm::vec3 p, glm::vec3 colour = {});
	void reset()
	{
		_assigned = false;
		clearVertices();
	}
	
	void setJob(const std::function<void(double, double)> &job)
	{
		_doJob = job;
	}
	
	virtual void drag(double x, double y);
	virtual bool mouseOver();
	virtual void unMouseOver();
private:
	void addThickLine(glm::vec3 start, glm::vec3 dir, glm::vec3 colour);
	void setupHarmonics();

	glm::vec3 _last{};
	bool _assigned = false;
	
	std::function<void(double, double)> _doJob;
};

#endif
