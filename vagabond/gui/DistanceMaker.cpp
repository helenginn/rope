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

#include "DistanceMaker.h"
#include <vagabond/core/Residue.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/gui/MetadataView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <iostream>

DistanceMaker::DistanceMaker(Scene *prev, IndexedSequence *sequence)
: SequenceView(prev, sequence)
{

}

void DistanceMaker::wipe()
{
	if (_modeButton != nullptr)
	{
		removeObject(_modeButton);
		delete _modeButton;
		_modeButton = nullptr;
	}

	_first = "";
	_second = "";
	_third = "";
	_aRes = nullptr;
	_bRes = nullptr;
	_cRes = nullptr;
	_stage = Nothing;
}

void DistanceMaker::angleButton()
{
	wipe();

	ImageButton *angle = new ImageButton("assets/images/protractor.png", this);
	angle->resize(0.06);
	angle->setCentre(0.9, 0.1);
	angle->setReturnTag("angle");
	angle->addAltTag("measuring angles");
	addObject(angle);
	_modeButton = angle;
	_mode = Angle;
}

void DistanceMaker::distanceButton()
{
	wipe();

	ImageButton *ruler = new ImageButton("assets/images/ruler.png", this);
	ruler->resize(0.03);
	ruler->setReturnTag("ruler");
	ruler->addAltTag("measuring distances");
	ruler->setCentre(0.9, 0.1);
	addObject(ruler);
	_modeButton = ruler;
	_mode = Ruler;
}

void DistanceMaker::setup()
{
	SequenceView::setup();

	distanceButton();
}

void DistanceMaker::addExtras(TextButton *t, Residue *r) 
{
	std::string tag = r->desc();
	t->addAltTag(tag);
	t->setInert(false);
}

void DistanceMaker::handleResidue(Button *button, Residue *r)
{
	_curr = r;
	glm::vec2 c = button->xy();
	Menu *m = new Menu(this, "atomname");
	
	for (const std::string &name : r->atomNames())
	{
		if (name.length() == 0 || name[0] == 'H')
		{
			continue;
		}

		m->addOption(name);
	}

	m->setup(c.x, c.y);
	setModal(m);
}

void DistanceMaker::handleAtomName(std::string name)
{
	_candidate = name;
	std::string mesg;

	if (_mode == Ruler)
	{
		if (_stage == Nothing)
		{
			mesg = "Measure distance from atom " + _curr->desc() + name + "?";
		}
		else if (_stage == ChosenFirst)
		{
			mesg = "Measure distance to atom " + _curr->desc() + name + "?";
		}

		std::cout << mesg << std::endl;

		AskYesNo *ayn = new AskYesNo(this, mesg, "confirm_atom", this);
		setModal(ayn);
	}

	if (_mode == Angle)
	{
		if (_stage == Nothing)
		{
			mesg = "Measure angle from atom " + _curr->desc() + name + "?";
		}
		else if (_stage == ChosenFirst)
		{
			mesg = "Measure angle through atom " + _curr->desc() + name + "?";
		}
		else if (_stage == ChosenSecond)
		{
			mesg = "Measure angle to atom " + _curr->desc() + name + "?";
		}

		std::cout << mesg << std::endl;

		AskYesNo *ayn = new AskYesNo(this, mesg, "confirm_atom", this);
		setModal(ayn);
	}

}

void DistanceMaker::confirmAtom()
{
	if (_stage == Nothing)
	{
		_first = _candidate;
		_aRes = _curr;
		_stage = ChosenFirst;
	}
	else if (_stage == ChosenFirst)
	{
		_second = _candidate;
		_bRes = _curr;
		_stage = ChosenSecond;
		
		if (_mode == Ruler)
		{
			calculateDistance();
		}
	}
	else if (_stage == ChosenSecond)
	{
		_third = _candidate;
		_cRes = _curr;

		if (_mode == Angle)
		{
			calculateAngle();
		}
	}
}

void DistanceMaker::calculateAngle()
{
	AtomRecall one(_aRes, _first);
	AtomRecall two(_bRes, _second);
	AtomRecall three(_cRes, _third);
	Metadata *md = _entity->angleBetweenAtoms(one, two, three);
	
	MetadataView *mv = new MetadataView(this, md);
	mv->show();
	_stage = Nothing;
}

void DistanceMaker::calculateDistance()
{
	AtomRecall one(_aRes, _first);
	AtomRecall two(_bRes, _second);
	Metadata *md = _entity->distanceBetweenAtoms(one, two);
	
	MetadataView *mv = new MetadataView(this, md);
	mv->show();
	_stage = Nothing;
}

void DistanceMaker::buttonPressed(std::string tag, Button *button)
{
	if (tag == "ruler")
	{
		angleButton();
	}

	if (tag == "angle")
	{
		distanceButton();
	}

	std::string atom = Button::tagEnd(tag, "atomname_");
	
	if (atom.length())
	{
		handleAtomName(atom);
		return;
	}
	
	std::string yes = Button::tagEnd(tag, "yes_");
	std::cout << "Yes: " << yes << std::endl;
	
	if (yes == "confirm_atom")
	{
		confirmAtom();
		return;
	}

	SequenceView::buttonPressed(tag, button);
}

void DistanceMaker::setEntity(Entity *ent)
{
	_entity = Environment::entityManager()->entity(ent->name());
	std::cout << "Entity ptr: " << _entity << std::endl;

}
