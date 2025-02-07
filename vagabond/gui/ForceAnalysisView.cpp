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

#include "ForceAnalysisView.h"
#include "DisplayUnit.h"
#include "AtomContent.h"
#include "ShowTorque.h"
#include "ShowForce.h"

#include <vagabond/core/forces/Torque.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/VisualPreferences.h>

ForceAnalysisView::ForceAnalysisView(Scene *prev, AtomContent *atoms)
: Scene(prev), Display(prev), _analysis(atoms)
{
	DisplayUnit *unit = new DisplayUnit(this);
	unit->loadAtoms(atoms);
	unit->displayAtoms();
	unit->startWatch();

	VisualPreferences vp{};
	vp.setCAlphaTrace(false);
	unit->setVisuals(vp);

	this->addDisplayUnit(unit);

	setPingPong(true);
}

void ForceAnalysisView::updateForces()
{
	for (ShowAbstractForce *show : _shows)
	{
		show->update();
	}
}

void ForceAnalysisView::setup()
{
	auto thandle = [this](Particle *p, Rod *rod, Torque *torque, float magnitude)
	{
		handleTorque(p, rod, torque, magnitude);
	};

	auto fhandle = [this](Particle *p, Force *force, float magnitude)
	{
		handleForce(p, force, magnitude);
	};

	_analysis.setHandleTorque(thandle);
	_analysis.setHandleForce(fhandle);
	_analysis.convert();
	
	TickBoxes *tickboxes = new TickBoxes(this, this);

	auto toggle_type = [this, tickboxes]
	(std::string tag, AbstractForce::Reason reason)
	{
		return [this, tickboxes, tag, reason]()
		{
			bool ticked = tickboxes->isTicked(tag);
			_analysis.toggleReason(reason, ticked);
			updateForces();
		};
	};
	
	tickboxes->addOption("Bond lengths", 
	                     toggle_type("Bond lengths", 
	                     AbstractForce::ReasonBondLength), true);
	tickboxes->addOption("Bond angles", 
	                     toggle_type("Bond angles", 
	                     AbstractForce::ReasonBondAngle), true);
	tickboxes->addOption("VdW contacts", 
	                     toggle_type("VdW contacts", 
	                     AbstractForce::ReasonVdwContact), true);
	tickboxes->addOption("Electrostatic contacts", 
	                     toggle_type("Electrostatic contacts", 
	                     AbstractForce::ReasonElectrostaticContact), true);
	tickboxes->addOption("Torsion angles", 
	                     toggle_type("Torsion angles", 
	                     AbstractForce::ReasonBondTorsion), true);
	tickboxes->setVertical(true);
	tickboxes->setOneOnly(false);
	tickboxes->arrange(0.8, 0.2, 1.02, 0.52);
	addObject(tickboxes);
	
	TextButton *calculate = new TextButton("Calculate", this);
	calculate->setLeft(0.8, 0.52);
	calculate->setReturnJob([this]()
					  {
						 _analysis.calculateUnknown();
			             updateForces();
					  });
	addObject(calculate);

	Display::setup();
}

void ForceAnalysisView::handleTorque(Particle *p, Rod *rod, 
                                     Torque *torque, float magnitude)
{
	ShowTorque *st = new ShowTorque(p, rod, torque, magnitude);
	addObject((Image *)st);
	_shows.push_back(st);
}

void ForceAnalysisView::handleForce(Particle *p, Force *force, float magnitude)
{
	ShowForce *sf = new ShowForce(p, force, magnitude);
	addObject((Image *)sf);
	_shows.push_back(sf);
}
