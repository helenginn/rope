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

#include <vagabond/core/protonic/CountProbe.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/Guilt.h>
#include <vagabond/gui/elements/Menu.h>
#include "ProbeCharge.h"
#include "ProtonNetworkView.h"

ProbeCharge::ProbeCharge(ProtonNetworkView *view, CountProbe *probe)
: FloatingImage("assets/images/" + probe->display() + ".png", 1, probe->_offset) 
{
	setQuickSwitch({"assets/images/circle_plus.png",
		            "assets/images/circle_2plus.png",
		            "assets/images/circle_3plus.png",
		            "assets/images/circle_minus.png",
		            "assets/images/circle_2minus.png",
		            "assets/images/circle_3minus.png",
		            "assets/images/circle_question.png",
		            "assets/images/transparency.png"});

	// FloatingImage draws with floating_box.fsh, which writes ValIndex = 0
	// for every fragment - nothing in the GPU pick buffer to click on. The
	// pick id this stamps into extra[3] (see reindex()) only reaches the
	// buffer through indexed_box.fsh, which writes ValIndex = vExtra[3] -
	// the same shader ProbeAtomStandaloneText (and the atom batch, via its
	// atlas variant) switch to. floating_box.vsh, shared with those, already
	// passes extra through as vExtra. Qualified, as everywhere else in this
	// class, because FloatingImage and IndexResponder both reach Renderable.
#ifndef __EMSCRIPTEN__
	FloatingImage::setFragmentShaderFile("assets/shaders/indexed_box.fsh");
#endif

	_view = view;
	_probe = probe;

	_probe->_obj.set_update([this](bool thorough)
	                       { thorough ? fullUpdate() : quickUpdate(); });
	_probe->_exist.add_update([this](bool thorough)
	                          { thorough ? fullUpdate() : quickUpdate(); });
	fullUpdate();
}

void ProbeCharge::quickUpdate()
{
	FloatingImage::addMainThreadJob
	([this]()
	{
		FloatingImage::changeImage("assets/images/" + _probe->display() + ".png");
		FloatingImage::setAlpha(_probe->alpha());
		FloatingImage::forceRender(true, true);
	});
}

void ProbeCharge::fullUpdate()
{
	FloatingImage::setPosition(_probe->position());
	FloatingImage::changeImage("assets/images/" + _probe->display() + ".png");
	FloatingImage::setAlpha(_probe->alpha());
	FloatingImage::forceRender(true, true);
}

void ProbeCharge::selected(int idx, bool inverse)
{
	_selected = !inverse;

	// same additive highlight tint as ProbeBond::selected() - color[3]
	// (set via setAlpha elsewhere) is a separate blend-alpha for this
	// tint, not the texture's opacity, so don't touch it here; only the
	// rgb tint itself toggles.
	float colour = (inverse ? 0.f : 0.3f);
	FloatingImage::setColour(colour, colour, colour * 2.f);
	FloatingImage::forceRender(true, false);
}


void ProbeCharge::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingImage::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		FloatingImage::_vertices[i].extra[3] = offset + 1.5;
	}

}

namespace
{

// "+1", "-2", "0" - how a charge is named in the menu and the undo message.
std::string charge_label(int charge)
{
	if (charge > 0)
	{
		return "+" + std::to_string(charge);
	}
	return std::to_string(charge);
}

// the tag prefix buttonPressed() recognises; the rest is the integer charge.
const std::string chargeTag = "setC_";

}

void ProbeCharge::offerChargeMenu()
{
	// every charge this probe can still take - like ProbeBond's menu, there
	// is nothing to choose once only one is left.
	std::vector<hnet::Count::Values> options = _probe->_obj.values();
	if (options.size() < 2)
	{
		return;
	}

	Menu *m = new Menu(_view, this);

	for (const hnet::Count::Values &option : options)
	{
		int charge = hnet::count_to_int(option);
		m->addOption("Charge " + charge_label(charge),
		             chargeTag + std::to_string(charge));
	}

	_view->setMenu(m);
}

void ProbeCharge::interacted(int idx, bool hover, bool left)
{
	// right click, as for ProbeBond and ProbeAtom
	if (!hover && !left)
	{
		offerChargeMenu();
	}
}

void ProbeCharge::declareCharge(hnet::Count::Values value)
{
	GuiltVersion gv = Guilt::issueNext();
	CountProbe *probe = _probe;
	ProtonNetworkView *view = _view;
	std::string label = charge_label(hnet::count_to_int(value));

	auto make_declaration = [probe, view, gv, value, label]
	{
		bool okay = probe->_obj.assign_value_and_check(value, gv);
		std::cout << "Declared " << probe->_obj.desc() << " charge " << label
		          << ", OK: " << (okay ? "YES" : "NO") << std::endl;
		if (!okay)
		{
			view->setInformation("Contradiction occurred in logical "\
			                     "network!!\nCtrl+Z to undo");
		}
		hnet::ConnectBase::_silent = false;
	};

	auto rescind_declaration = [probe, gv]
	{
		probe->_obj.forget_all(gv);
		probe->_obj.check_all(gv);
	};

	_view->network().undoStack().addJobAndExecute(make_declaration,
	                                              rescind_declaration,
	                                              "Declare charge " + label);
}

void ProbeCharge::buttonPressed(std::string tag, Button *button)
{
	if (tag.compare(0, chargeTag.length(), chargeTag) != 0)
	{
		return;
	}

	int charge = std::stoi(tag.substr(chargeTag.length()));

	// the same options the menu was built from, matched back by charge
	for (const hnet::Count::Values &option : _probe->_obj.values())
	{
		if (hnet::count_to_int(option) == charge)
		{
			declareCharge(option);
			return;
		}
	}
}
