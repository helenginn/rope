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

#include <vagabond/core/protonic/CovalentProbe.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/protonic/hnet.h>
#include <vagabond/gui/elements/Menu.h>
#include <sstream>
#include "ProtonNetworkView.h"
#include "ProbeAtom.h"
#include "ProbeBond.h"

using namespace hnet;

template <class Value>
void populate_menu(Menu *m, const std::vector<Value> &options)
{
	if (options.size() > 1)
	{
		for (const Value &option : options)
		{
			std::ostringstream ss;
			ss << option;
			m->addOption(ss.str(), "setB_" + ss.str());
		}
	}
}

void ProbeBond::offerBondMenu()
{
	Menu *m = new Menu(_view, this);

	if (!_probe->is_covalent())
	{
		std::vector<Bond::Values> options = _probe->_obj.values();
		populate_menu(m, options);
	}
	else
	{
		CovalentProbe *cp = static_cast<CovalentProbe *>(_probe);
		std::vector<Covalent::Values> options = cp->_cov.values();
		populate_menu(m, options);
	}

	{
		std::vector<Existence::Values> options = _probe->_exist.values();
		populate_menu(m, options);
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
	if (!_leftGui || !_rightGui)
	{
		return;
	}

	std::string lres = _probe->_left.display();
	if (lres == "") return;
	std::string rres = _probe->_right.display();
	if (rres == "") return;

	// follow wherever the endpoints are currently rendered, not the
	// physics probe position directly - see setEndpoints().
	glm::vec3 start = _leftGui->FloatingText::centroid();
	glm::vec3 end = _rightGui->FloatingText::centroid();
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
	Image::addMainThreadJob
	([this]()
	{
		updateProbe();
		updatePosition();
	});
}

ProbeBond::ProbeBond(ProtonNetworkView *view, BondProbe *probe)
: BondRod("assets/images/" + probe->display() + ".png")
{
	setQuickSwitch({"assets/images/weak_bond.png",
		            "assets/images/strong_bond.png",
		            "assets/images/transparency.png",
		            "assets/images/present_bond.png",
		            "assets/images/lone_pair.png",
		            "assets/images/unassigned_bond.png"});
	_view = view;
	_probe = probe;

	fullUpdate();
	
	auto update_method = [this](bool thorough)
	{
		thorough ? fullUpdate() : updateProbe();
	};

	_probe->_obj.set_update(update_method);
	_probe->existence().set_update(update_method);
	
	if (_probe->is_covalent())
	{
		CovalentProbe *cp = static_cast<CovalentProbe *>(_probe);
		cp->_cov.set_update(update_method);

	}
}

template <class Object, class Value>
void declareBondValue(ProbeBond *me, Object &pb, const Value &value)
{
	std::string name = "Declare bond existence";
	GuiltVersion gv = Guilt::issueNext();

	std::ostringstream ss;

	auto make_declaration = [&pb, gv, me, value]
	{
		bool okay = pb.assign_value_and_check(value, gv);
		std::cout << "Declared " << pb.desc() << " existence, ";
		std::cout << "OK: " << (okay ? "YES" : "NO") << std::endl;
		if (!okay)
		{
			me->view()->setInformation("Contradiction occurred in logical "\
			                           "network!!\nCtrl+Z to undo");
		}
		ConnectBase::_silent = false;
	};

	auto rescind_declaration = [gv, &pb]
	{
		pb.forget_all(gv);
		pb.check_all(gv);
	};

	ss << name << " " << value << std::endl;
	std::string message = ss.str();
	
	me->view()->network().undoStack().addJobAndExecute(make_declaration,
	                                                   rescind_declaration,
	                                                   message);
}

void ProbeBond::buttonPressed(std::string tag, Button *button)
{
	if (tag == "setB_LonePair")
	{
		declareBondValue(this, _probe->_obj, Bond::LonePair);
	}
	else if (tag == "setB_Acceptor")
	{
		declareBondValue(this, _probe->_obj, Bond::Weak);
	}
	else if (tag == "setB_Donor")
	{
		declareBondValue(this, _probe->_obj, Bond::Strong);
	}
	else if (tag == "setB_Broken")
	{
		declareBondValue(this, _probe->_obj, Bond::Broken);
	}
	else if (tag == "setB_Absent")
	{
		declareBondValue(this, _probe->_exist, Existence::Absent);
	}
	else if (tag == "setB_Present")
	{
		declareBondValue(this, _probe->_exist, Existence::Present);
		declareBondValue(this, _probe->_obj, Bond::NotBroken);
	}
	else if (tag == "setB_Single")
	{
		CovalentProbe *cp = static_cast<CovalentProbe *>(_probe);
		declareBondValue(this, cp->_cov, Covalent::Single);
	}
	else if (tag == "setB_Double")
	{
		CovalentProbe *cp = static_cast<CovalentProbe *>(_probe);
		declareBondValue(this, cp->_cov, Covalent::Double);
	}

}

void ProbeBond::selected(int idx, bool inverse)
{
	_selected = !inverse;
	float colour = (inverse ? 0.f : 0.3f);
	Image::setColour(colour, colour, colour * 2.f);
	Image::forceRender(true, false);
}
