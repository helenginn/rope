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

#include "ProbeAtom.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/Decree.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/gui/elements/Menu.h>

using namespace hnet;

void ProbeAtom::fullUpdate()
{
	glm::vec3 c = _probe->colour();
	FloatingText::setText(_probe->display());
	FloatingText::correctBox(_probe->mult(), 0.0);
	FloatingText::setPosition(_probe->position());
	FloatingText::setColour(c.x, c.y, c.z);
	FloatingText::setAlpha(_probe->alpha());
	updateProbe();
	FloatingText::forceRender(true, true);
}

void ProbeAtom::updatePosition()
{
	FloatingText::setPosition(_probe->position());
	FloatingText::forceRender(true, false);
}

void ProbeAtom::updateProbe()
{
	FloatingText::addMainThreadJob
	([this]()
	{
		FloatingText::changeText(_probe->display());
		FloatingText::setAlpha(_probe->alpha());
	});
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, AtomProbe *probe)
: FloatingText(probe->display(), probe->mult(), 0.0)
{
	FloatingText::setPosition(probe->position());
	_probe = probe;
	_view = view;
	
#ifndef __EMSCRIPTEN__
	std::string shader = "assets/shaders/indexed_box.fsh";
#else
	std::string shader = "assets/shaders/box.fsh";
#endif

	FloatingText::setFragmentShaderFile(shader);
	probe->_obj.set_update([this]() { updateProbe(); });
	probe->existence().set_update([this]() { updateProbe(); });
	fullUpdate();
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, HydrogenProbe *probe)
: FloatingText(probe->display(), 25, 0.0)
{
	_view = view;
	_probe = probe;

	FloatingText::setPosition(probe->position());
	FloatingText::setAlpha(probe->alpha());
	FloatingText::setColour(0, 0, 0);

#ifndef __EMSCRIPTEN__
	std::string shader = "assets/shaders/indexed_box.fsh";
#else
	std::string shader = "assets/shaders/box.fsh";
#endif
	FloatingText::setFragmentShaderFile(shader);
	probe->_obj.set_update([this]() { updateProbe(); });
	probe->existence().set_update([this]() { updateProbe(); });

	fullUpdate();
}

void ProbeAtom::hoverOverAtom()
{
	if (_probe->is_atom())
	{
		AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);
		::Atom *atom = aProbe->atom();
		std::string conf; conf += aProbe->_conf;
		_view->setInformation(atom->desc() + "," + conf);
	}
}

void ProbeAtom::offerHeavyAtomMenu()
{
	AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);
	if (aProbe->existence().is_certain())
	{
		return;
	}

	std::vector<Existence::Values> options = aProbe->existence().values();

	Menu *m = new Menu(_view, this);
	
	for (const Existence::Values &option : options)
	{
		std::ostringstream ss;
		ss << option;
		m->addOption(ss.str(), [this, option]
		             () { declareAtomExistence(option); });
	}

	_view->setMenu(m);
}

void ProbeAtom::offerHydrogenMenu()
{
	HydrogenProbe *hProbe = static_cast<HydrogenProbe *>(_probe);

	std::vector<Existence::Values> options = hProbe->_obj.values();

	Menu *m = new Menu(_view, this);
	
	for (const Existence::Values &option : options)
	{
		std::ostringstream ss;
		ss << option;
		m->addOption(ss.str(), "setH_" + ss.str());
	}

	_view->setMenu(m);
}

void ProbeAtom::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		FloatingText::setHighlighted(true);
		_view->setManualAdjust(this);
		_view->setActive((FloatingText *)this);
		hoverOverAtom();
	}

	if (!left && !hover && !_probe->is_atom())
	{
		offerHydrogenMenu();
	}
	else if (!left && !hover && _probe->is_atom())
	{
		offerHeavyAtomMenu();
	}

}

void ProbeAtom::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingText::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		FloatingText::_vertices[i].extra[3] = offset + 1.5;
	}
}

void ProbeAtom::selected(int idx, bool inverse)
{
	_selected = !inverse;

	if (!_selected)
	{
		glm::vec3 c = _probe->colour();
		FloatingText::setColour(c.x, c.y, c.z);
	}
	else
	{
		glm::vec3 c = _probe->colour();
		FloatingText::setColour(c.x + 0.5, c.y + 0.5, c.z + 0.5);
	}

	FloatingText::forceRender(true, false);
}

void ProbeAtom::declareAtomExistence(Existence::Values value)
{
	std::string name = "Declare atom";
	GuiltVersion gv = Guilt::issueNext();
	std::string present = (value == Existence::Present ? "present" : "absent");
	std::string message = "Declare atom " + present;

	auto make_declaration = [gv, value, message, this]
	{
		AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);
		bool okay = aProbe->existence().assign_value_and_check(value, gv);
		std::cout << "OK: " << okay << std::endl;
		if (!okay)
		{
			_view->setInformation("Contradiction occurred in logical "\
			                      "network!!\nCtrl+Z to undo");
		}
	};

	auto rescind_declaration = [gv, this]
	{
		AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);

		aProbe->existence().forget_all(gv);
		aProbe->existence().check_all(gv);
	};
	
	_view->network().undoStack().addJobAndExecute(make_declaration,
	                                              rescind_declaration,
	                                              message);
}

void ProbeAtom::declareHydrogen(Existence::Values value)
{
	std::string name = "Declare hydrogen";
	GuiltVersion gv = Guilt::issueNext();
	std::string present = (value == Existence::Present ? "present" : "absent");
	std::string message = "Declare hydrogen " + present;

	auto make_declaration = [gv, value, this]
	{
		HydrogenProbe *hProbe = static_cast<HydrogenProbe *>(_probe);
		bool okay = hProbe->_obj.assign_value_and_check(value, gv);
		std::cout << "OK: " << okay << std::endl;
		if (!okay)
		{
			_view->setInformation("Contradiction occurred in logical "\
			                      "network!!\nCtrl+Z to undo");
		}
	};

	auto rescind_declaration = [gv, this]
	{
		HydrogenProbe *hProbe = static_cast<HydrogenProbe *>(_probe);

		hProbe->_obj.forget_all(gv);
		hProbe->_obj.check_all(gv);
	};
	
	_view->network().undoStack().addJobAndExecute(make_declaration,
	                                              rescind_declaration,
	                                              message);
}

void ProbeAtom::buttonPressed(std::string tag, Button *button)
{
	if (tag == "setH_Present")
	{
		declareHydrogen(Existence::Present);
	}
	else if (tag == "setH_Absent")
	{
		declareHydrogen(Existence::Absent);
	}

}

void ProbeAtom::extraUniforms()
{
	glm::vec4 glow = _probe->glow();
	GLint uGlow = 
	glGetUniformLocation(FloatingText::_program, "aGlow");
	glUniform4f(uGlow, glow[0], glow[1], glow[2], glow[3]);
	FloatingText::_gl->checkErrors("glowing");
	FloatingText::extraUniforms();

}
