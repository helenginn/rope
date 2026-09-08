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

ProbeAtomStandaloneText::ProbeAtomStandaloneText(ProbeAtom *owner,
                                                const std::string &text,
                                                float mult, float yOff)
: FloatingText(text, mult, yOff), _owner(owner)
{
#ifndef __EMSCRIPTEN__
	std::string shader = "assets/shaders/indexed_box.fsh";
#else
	std::string shader = "assets/shaders/box.fsh";
#endif
	FloatingText::setFragmentShaderFile(shader);
}

void ProbeAtomStandaloneText::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingText::vertexCount(); i++)
	{
		FloatingText::_vertices[i].extra[3] = offset + 1.5;
	}
}

void ProbeAtomStandaloneText::interacted(int idx, bool hover, bool left)
{
	_owner->interacted(idx, hover, left);
}

void ProbeAtomStandaloneText::selected(int idx, bool inverse)
{
	_owner->selected(idx, inverse);
}

void ProbeAtom::fullUpdate()
{
	auto job = [this]() { fullUpdateProbe(); };

	if (isStandalone())
	{
		_standalone->addMainThreadJob(job);
	}
	else
	{
		_batch->addMainThreadJob(job);
	}
}

void ProbeAtom::fullUpdateProbe()
{
	glm::vec3 c = _probe->colour();

	if (isStandalone())
	{
		_standalone->setText(_probe->display());
		_standalone->correctBox(_probe->mult(), 0.0);
		_standalone->setPosition(_probe->position());
		_standalone->setColour(c.x, c.y, c.z);
		_standalone->setAlpha(_probe->alpha());
		_standalone->forceRender(true, true);
	}
	else
	{
		_batch->setSlotFull(_slot, _probe->display(), _probe->mult(),
		                    _probe->position());
		_batch->setSlotColour(_slot, c.x, c.y, c.z);
		_batch->setSlotAlpha(_slot, _probe->alpha());
	}
}

void ProbeAtom::updatePosition()
{
	if (isStandalone())
	{
		_standalone->setPosition(_probe->position());
		_standalone->forceRender(true, false);
	}
	else
	{
		_batch->setSlotPosition(_slot, _probe->position());
	}
}

void ProbeAtom::updateProbe()
{
	auto job = [this]()
	{
		if (isStandalone())
		{
			_standalone->changeText(_probe->display());
			_standalone->setAlpha(_probe->alpha());
		}
		else
		{
			_batch->changeSlotText(_slot, _probe->display());
			_batch->setSlotAlpha(_slot, _probe->alpha());
		}
	};

	if (isStandalone())
	{
		_standalone->addMainThreadJob(job);
	}
	else
	{
		_batch->addMainThreadJob(job);
	}
}

glm::vec3 ProbeAtom::currentPosition() const
{
	if (isStandalone())
	{
		return _standalone->centroid();
	}

	return _batch->slotPosition(_slot);
}

void ProbeAtom::setRenderPosition(const glm::vec3 &pos)
{
	if (isStandalone())
	{
		_standalone->setPosition(pos);
	}
	else
	{
		_batch->setSlotPosition(_slot, pos);
	}
}

void ProbeAtom::setRenderAlpha(double alpha)
{
	if (isStandalone())
	{
		_standalone->setAlpha(alpha);
	}
	else
	{
		_batch->setSlotAlpha(_slot, alpha);
	}
}

void ProbeAtom::forceRedraw()
{
	if (isStandalone())
	{
		_standalone->forceRender(true, false);
	}
	else
	{
		_batch->forceRender(true, false);
	}
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
                    AtomProbe *probe)
{
	_batch = batch;
	_view = view;
	_probe = probe;

	if (probe->_text.length())
	{
		// a reporter atom (see ProbeAtomStandaloneText's header comment)
		// - display() will return this fixed text forever, never one of
		// ProbeAtomBatch's atlas states, so it gets its own individual
		// Renderable instead of a batch slot.
		_standalone = new ProbeAtomStandaloneText(this, probe->display(),
		                                          probe->mult(), 0.0);
		view->addObject((FloatingText *)_standalone);
		view->addIndexResponder(_standalone);
	}
	else
	{
		_slot = _batch->appendAtom(this);
	}

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

ProbeAtom::~ProbeAtom()
{
	if (_standalone)
	{
		_view->removeObject((FloatingText *)_standalone);
		delete _standalone;
		_standalone = nullptr;
	}
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

	glm::vec3 c = _probe->colour();
	if (_selected)
	{
		c += 0.5f;
	}

	if (isStandalone())
	{
		_standalone->setColour(c.x, c.y, c.z);
	}
	else
	{
		_batch->setSlotColour(_slot, c.x, c.y, c.z);
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
