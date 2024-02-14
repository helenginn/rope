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

#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/protonic/hnet.h>
#include <vagabond/gui/elements/Menu.h>
#include "ProtonNetworkView.h"
#include "ProbeBond.h"

using namespace hnet;

void ProbeBond::fixVertices(const glm::vec3 &start, const glm::vec3 &dir)
{
	Image::clearVertices();

	{
		Snow::Vertex &v = Image::addVertex(start);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = Image::addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 1;
	}

	{
		Snow::Vertex &v = Image::addVertex(start);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = Image::addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 1;
	}

	Image::addIndices(-4, -3, -2);
	Image::addIndices(-3, -2, -1);

}

void ProbeBond::offerBondMenu()
{
	std::vector<Bond::Values> options = _probe->_obj.values();

	Menu *m = new Menu(_view, this);
	
	for (const Bond::Values &option : options)
	{
		std::ostringstream ss;
		ss << option;
		m->addOption(ss.str(), "setB_" + ss.str());
	}

	_view->setMenu(m);

}

void ProbeBond::interacted(int idx, bool hover, bool left)
{
	if (!hover && !left)
	{
		offerBondMenu();
	}
}

void ProbeBond::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < Image::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		Image::_vertices[i].extra[3] = offset + 1.5;
	}
}

size_t ProbeBond::requestedIndices()
{
	return Image::vertexCount();
}

void ProbeBond::updateProbe()
{
	Image::setAlpha(_probe->alpha());
	changeImage("assets/images/" + _probe->display() + ".png");
}

ProbeBond::ProbeBond(ProtonNetworkView *view, BondProbe *probe)
: Image("assets/images/" + probe->display() + ".png")
{
	_view = view;
	_probe = probe;

	Image::setVertexShaderFile("assets/shaders/axes.vsh");
	Image::setFragmentShaderFile("assets/shaders/axes.fsh");
	Image::setUsesProjection(true);

	glm::vec3 start = _probe->position();
	glm::vec3 end = _probe->end();
	glm::vec3 dir = end - start;
	dir /= 4.f;
	fixVertices(start + dir, end - start - 2.f * dir);
	
	updateProbe();
	_probe->_obj.set_update([this](){ updateProbe(); });
}

void ProbeBond::declareBond(Bond::Values value)
{
	std::string name = "Declare bond";
	Decree *d = _view->network().newDecree(name);

	_probe->_obj.assign_value(value, d, d);
}

void ProbeBond::buttonPressed(std::string tag, Button *button)
{
	if (tag == "setB_Absent")
	{
		declareBond(Bond::Absent);
	}
	else if (tag == "setB_Weak")
	{
		declareBond(Bond::Weak);
	}
	else if (tag == "setB_Strong")
	{
		declareBond(Bond::Strong);
	}
	else if (tag == "setB_Broken")
	{
		declareBond(Bond::Broken);
	}

}
