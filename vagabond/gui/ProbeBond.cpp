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
#include <sstream>
#include "ProtonNetworkView.h"
#include "ProbeBond.h"

using namespace hnet;

void ProbeBond::fixVertices(const glm::vec3 &start, const glm::vec3 &dir)
{
	Image::_vertices.resize(4);

	{
		Snow::Vertex &v = Image::_vertices[0];
		v.pos = start;
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = Image::_vertices[1];
		v.pos = start + dir;
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 1;
	}

	{
		Snow::Vertex &v = Image::_vertices[2];
		v.pos = start;
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = Image::_vertices[3];
		v.pos = start + dir;
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 1;
	}

	if (Image::_indices.size() == 0)
	{
		Image::addIndices(-4, -3, -2);
		Image::addIndices(-3, -2, -1);
	}
}

void ProbeBond::offerBondMenu()
{
	Menu *m = new Menu(_view, this);

	{
		std::vector<Bond::Values> options = _probe->_obj.values();
		if (options.size() > 1)
		{
			for (const Bond::Values &option : options)
			{
				std::ostringstream ss;
				ss << option;
				m->addOption(ss.str(), "setB_" + ss.str());
			}
		}
	}

	{
		std::vector<Existence::Values> options = _probe->_exist.values();
		if (options.size() > 1)
		{
			for (const Existence::Values &option : options)
			{
				std::ostringstream ss;
				ss << option;
				m->addOption(ss.str(), "setB_" + ss.str());
			}
		}
	}

	_view->setMenu(m);

}

void ProbeBond::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		Probe *pr = &_probe->left();
		_view->setManualAdjust(pr);
	
	}
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
	std::string result = _probe->display();
	if (result == "")
	{
		return;
	}
	changeImage("assets/images/" + result + ".png");
	Image::setAlpha(_probe->alpha());
}

void ProbeBond::updatePosition()
{
	std::string lres = _probe->_left.display();
	if (lres == "") return;
	std::string rres = _probe->_right.display();
	if (rres == "") return;

	glm::vec3 start = _probe->position();
	glm::vec3 end = _probe->end();
	glm::vec3 truncate = (end - start) / 4.f;
	
	float left = 0;
	float right = 0;
	if (lres != " ") {left += 1; right -= 1; }
	if (rres != " ") { right -= 1; }

	fixVertices(start + truncate * left, 
	            end - start + truncate * right);

	Image::forceRender(true, true);
}

void ProbeBond::fullUpdate()
{
	updateProbe();
	updatePosition();
	
}

ProbeBond::ProbeBond(ProtonNetworkView *view, BondProbe *probe)
: Image("assets/images/" + probe->display() + ".png")
{
	setQuickSwitch({"assets/images/weak_bond.png", 
		            "assets/images/strong_bond.png", 
		            "assets/images/transparency.png", 
		            "assets/images/present_bond.png", 
		            "assets/images/unassigned_bond.png"});
	_view = view;
	_probe = probe;

	Image::setVertexShaderFile("assets/shaders/axes.vsh");
	Image::setFragmentShaderFile("assets/shaders/axes.fsh");
	Image::setUsesProjection(true);

	fullUpdate();

	_probe->_obj.set_update([this](){ updateProbe(); });
	_probe->existence().set_update([this]() { updateProbe(); });
}

void ProbeBond::declareBondExistence(Existence::Values value)
{
	std::string name = "Declare bond existence";
	GuiltVersion gv = Guilt::issueNext();

	std::ostringstream ss;

	auto make_declaration = [gv, value, this]
	{
		bool okay = _probe->_exist.assign_value_and_check(value, gv);
		std::cout << "Declared " << _probe->desc() << " existence, ";
		std::cout << "OK: " << (okay ? "YES" : "NO") << std::endl;
		if (!okay)
		{
			_view->setInformation("Contradiction occurred in logical "\
			                      "network!!\nCtrl+Z to undo");
		}
	};

	auto rescind_declaration = [gv, this]
	{
		_probe->_obj.forget_all(gv);
		_probe->_obj.check_all(gv);
	};

	ss << name << " " << value << std::endl;
	std::string message = ss.str();
	
	_view->network().undoStack().addJobAndExecute(make_declaration,
	                                              rescind_declaration,
	                                              message);
}

void ProbeBond::declareBond(Bond::Values value)
{
	std::string name = "Declare bond";
	GuiltVersion gv = Guilt::issueNext();

//	_probe->_obj.assign_value(value, d, d);
	std::ostringstream ss;

	auto make_declaration = [gv, value, this]
	{
		bool okay = _probe->_obj.assign_value_and_check(value, gv);
		std::cout << "Declared " << _probe->desc() << ", ";
		std::cout << "OK: " << (okay ? "YES" : "NO") << std::endl;
		if (!okay)
		{
			_view->setInformation("Contradiction occurred in logical "\
			                      "network!!\nCtrl+Z to undo");
		}
	};

	auto rescind_declaration = [gv, this]
	{
		_probe->_obj.forget_all(gv);
		_probe->_obj.check_all(gv);
	};

	ss << name << " " << value << std::endl;
	std::string message = ss.str();
	
	_view->network().undoStack().addJobAndExecute(make_declaration,
	                                              rescind_declaration,
	                                              message);
}

void ProbeBond::buttonPressed(std::string tag, Button *button)
{
	if (tag == "setB_LonePair")
	{
		declareBond(Bond::LonePair);
	}
	else if (tag == "setB_Acceptor")
	{
		declareBond(Bond::Weak);
	}
	else if (tag == "setB_Donor")
	{
		declareBond(Bond::Strong);
	}
	else if (tag == "setB_Broken")
	{
		declareBond(Bond::Broken);
	}
	else if (tag == "setB_Absent")
	{
		declareBondExistence(Existence::Absent);
	}
	else if (tag == "setB_Present")
	{
		declareBondExistence(Existence::Present);
		declareBond(Bond::NotBroken);
	}

}

void ProbeBond::selected(int idx, bool inverse)
{
	_selected = !inverse;
	float colour = (inverse ? 0.f : 0.3f);
	Image::setColour(colour, colour, colour * 2.f);
	Image::forceRender(true, false);
}
