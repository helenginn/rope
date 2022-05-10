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

#ifndef __vagabond__ColourScheme__
#define __vagabond__ColourScheme__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/Rule.h>
#include <vagabond/utils/maths.h>

class ColourScheme : public Image
{
public:
	ColourScheme();
	ColourScheme(Scheme scheme);
	

	void addFixedPoint(float p, glm::vec4 colour);
	
	glm::vec4 colour(float p);
	
	/* create vertices/indices to display colour scheme */
	void setup();
private:
	bool sanitisePoints();
	void setScheme(Scheme scheme)
	{
		_scheme = scheme;
	}

	struct FixedPoint
	{
		float proportion;
		glm::vec4 colour;
		
		const bool operator<(const FixedPoint &other) const
		{
			return proportion < other.proportion;
		}
	};

	std::vector<FixedPoint> _points;

	Scheme _scheme;
};

#endif
