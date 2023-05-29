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

#include <iostream>
#include "ColourScheme.h"

ColourScheme::ColourScheme(Scheme scheme, bool vert)
: Image("assets/images/colour_scheme.png")
{
	clearVertices();
	setFragmentShaderFile("assets/shaders/multiply.fsh");
	_vert = vert;

	if (scheme == BlueOrange)
	{
		addFixedPoint(0.0, glm::vec4(0.32, 0.52, 0.92, 1.0));
		addFixedPoint(0.5, glm::vec4(0.6, 0.0, 0.09, 1.0));
		addFixedPoint(1.0, glm::vec4(0.86, 0.63, 0.13, 1.0));
		setScheme(BlueOrange);
	}
	else if (scheme == OrangeWhitePurple)
	{
		addFixedPoint(0.0, glm::vec4(0.73, 0.44, 0.09, 1.0));
		addFixedPoint(0.5, glm::vec4(1.0, 1.0, 1.0, 1.0));
		addFixedPoint(1.0, glm::vec4(0.44, 0.18, 0.53, 1.0));
		setScheme(OrangeWhitePurple);
	}
	else if (scheme == Heat)
	{
		addFixedPoint(0.0, glm::vec4(0.0, 0.0, 0.0, 1.0));
		addFixedPoint(0.1, glm::vec4(0.05, 0.05, 0.05, 1.0));
		addFixedPoint(0.4, glm::vec4(1.0, 0.05, 0.05, 1.0));
		addFixedPoint(0.7, glm::vec4(1.0, 1.0, 0.05, 1.0));
		addFixedPoint(1.0, glm::vec4(1.0, 1.0, 1.0, 1.0));
		setScheme(Heat);
	}

	setup();
}

ColourScheme::ColourScheme() : Image("assets/images/colour_scheme.png")
{
	clearVertices();
	setFragmentShaderFile("assets/shaders/multiply.fsh");

}

void ColourScheme::addFixedPoint(float p, glm::vec4 colour)
{
	if (p < 0 || p > 1)
	{
		throw std::runtime_error("Fixed point proportion out of range 0-1");
	}

	FixedPoint fp{p, colour};
	_points.push_back(fp);
	
	std::sort(_points.begin(), _points.end());
}

glm::vec4 colour_between(glm::vec4 last, glm::vec4 curr, float frac)
{
//	rgb_to_hsv(last.x, last.y, last.z);
//	rgb_to_hsv(curr.x, curr.y, curr.z);
	glm::vec4 ret = last;
	glm::vec4 diff = curr - last;
	diff *= frac;
	ret += diff;
//	hsv_to_rgb(ret.x, ret.y, ret.z);
	return ret;
}

glm::vec4 ColourScheme::colour(float p)
{
	if (p != p)
	{
		return glm::vec4(0.5, 0.5, 0.5, 0.2);
	}

	// check for boundaries //
	const FixedPoint *last = nullptr;

	for (const FixedPoint &fp : _points)
	{
		/* first fixed point is already beyond the requested p */
		if (!last && fp.proportion > p)
		{
			return fp.colour;
		}
		/* somewhere between the last and this one */
		else if (last && fp.proportion > p) 
		{
			double frac = (p - last->proportion);
			frac /= (fp.proportion - last->proportion);
			glm::vec4 result = colour_between(last->colour, fp.colour, frac);
			return result;
		}

		last = &fp;
	}
	
	/* if we got here, we never found the right point, so we take the
	 * last as the extremity */
	return last->colour;
}

bool ColourScheme::sanitisePoints()
{
	if (_points.size() < 2)
	{
		return false;
	}

	if (_points.front().proportion > 0)
	{
		_points.front().proportion = 0;
	}

	if (_points.back().proportion < 1)
	{
		_points.back().proportion = 1;
	}
	
	return true;
}

void stretch_p(float &p)
{
	p = p * 2 - 1;
}

void ColourScheme::setup()
{
	if (!sanitisePoints())
	{
		return;
	}

	for (const FixedPoint &fp : _points)
	{
		float p = fp.proportion;
		float orig = p;
		stretch_p(p);

		Snow::Vertex &lower = addVertex((_vert ? -0.1 : p), 
		                                (_vert ? p : -0.1), 0);
		lower.color = fp.colour;
		lower.tex = glm::vec2(orig, 0.0);
		Snow::Vertex &upper = addVertex((_vert ? +0.1 : p), 
		                                (_vert ? p : +0.1), 0);
		upper.color = fp.colour;
		upper.tex = glm::vec2(orig, 1.0);
	
		if (vertexCount() < 4)
		{
			continue;
		}
		
		addIndex(-2);
		addIndex(-3);
		addIndex(-4);
		addIndex(-1);
		addIndex(-2);
		addIndex(-3);
	}
	
}
