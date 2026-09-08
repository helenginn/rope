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
#include "ProbeAtomBatch.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/gui/elements/Menu.h>

using namespace hnet;

void ProbeAtom::fullUpdate()
{
	_batch->addMainThreadJob
	([this]()
	 {
		fullUpdateProbe();
	});
}

void ProbeAtom::fullUpdateProbe()
{
	glm::vec3 c = _probe->colour();
	_batch->setSlotFull(_slot, _probe->display(), _probe->mult(),
	                    _probe->position());
	_batch->setSlotColour(_slot, c.x, c.y, c.z);
	_batch->setSlotAlpha(_slot, _probe->alpha());
}

void ProbeAtom::updatePosition()
{
	_batch->setSlotPosition(_slot, _probe->position());
}

void ProbeAtom::updateProbe()
{
	_batch->addMainThreadJob
	([this]()
	{
		_batch->changeSlotText(_slot, _probe->display());
		_batch->setSlotAlpha(_slot, _probe->alpha());
	});
}

glm::vec3 ProbeAtom::currentPosition() const
{
	return _batch->slotPosition(_slot);
}

void ProbeAtom::setRenderPosition(const glm::vec3 &pos)
{
	_batch->setSlotPosition(_slot, pos);
}

void ProbeAtom::setRenderAlpha(double alpha)
{
	_batch->setSlotAlpha(_slot, alpha);
}

void ProbeAtom::forceRedraw()
{
	_batch->forceRender(true, false);
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
                    AtomProbe *probe)
{
	_batch = batch;
	_view = view;
	_probe = probe;
	_slot = _batch->appendAtom(this);

	probe->_obj.set_update([this](bool thorough)
	                       { thorough ? fullUpdate() : updateProbe(); });
	probe->existence().add_update([this](bool thorough) { updateProbe(); });

	fullUpdateProbe();
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
                    HydrogenProbe *probe)
{
	_batch = batch;
	_view = view;
	_probe = probe;
	_slot = _batch->appendAtom(this);

	probe->_obj.set_update([this](bool thorough)
	                       { thorough ? fullUpdate() : updateProbe(); });
	probe->existence().set_update([this](bool thorough) { updateProbe(); });

	fullUpdateProbe();
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

	std::vector<Existence::Values> options = aProbe->existence().values();

	Menu *m = new Menu(_view, this);

	if (!aProbe->existence().is_certain())
	{
		for (const Existence::Values &option : options)
		{
			std::ostringstream ss;
			ss << option;
			m->addOption(ss.str(), [this, option]
			             () { declareAtomExistence(option); });
		}
	}

	auto realign_atom = [aProbe]()
	{
		aProbe->realign();
	};

	m->addOption("realign", realign_atom);

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
		_view->setManualAdjust(this);
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

void ProbeAtom::selected(int idx, bool inverse)
{
	_selected = !inverse;

	if (!_selected)
	{
		glm::vec3 c = _probe->colour();
		_batch->setSlotColour(_slot, c.x, c.y, c.z);
	}
	else
	{
		glm::vec3 c = _probe->colour();
		_batch->setSlotColour(_slot, c.x + 0.5, c.y + 0.5, c.z + 0.5);
	}

	forceRedraw();
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
