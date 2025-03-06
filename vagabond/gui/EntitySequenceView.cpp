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

#include "Region.h"
#include "AtomRecall.h"
#include "RegionMenu.h"
#include "EntitySequenceView.h"
#include <vagabond/core/Residue.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/gui/MetadataView.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/AskForText.h>
#include <iostream>

EntitySequenceView::EntitySequenceView(Scene *prev, IndexedSequence *sequence)
: PickAtomFromSequence(prev, sequence)
{

}

void EntitySequenceView::wipe()
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

ImageButton *EntitySequenceView::addButton(const std::string &image,
                                           const EntitySequenceView::Mode 
                                           &mode, float resize, 
                                           const std::string &alt_tag)
{
	wipe();

	ImageButton *b = new ImageButton(image, this);
	b->resize(resize);
	b->setCentre(0.9, 0.1);
	b->addAltTag(alt_tag);
	addObject(b);
	_modeButton = b;
	_mode = mode;
	refresh();

	return b;
}

void EntitySequenceView::angleButton()
{
	ImageButton *b = addButton("assets/images/protractor.png", Angle,
	                           0.06, "measuring angles");
	b->setReturnJob([this]() { definingButton(); });
}

void EntitySequenceView::definingButton()
{
	ImageButton *b = addButton("assets/images/protein-coloured.png", Defining,
	                           0.08, "defining regions");
	b->setReturnJob([this]() { distanceButton(); });
	
	int num = _entity->regionManager().objectCount();
	if (num > 0)
	{
		std::string text = "See " + std::to_string(num) + " region";
		if (num > 1) { text += "s"; }
		TextButton *see = new TextButton(text, this);
		see->setCentre(0.9, 0.16);
		see->resize(0.6);
		auto make_menu = [this]()
		{
			RegionMenu *menu = new RegionMenu(this, &_entity->regionManager());
			menu->show();
		};
		see->setReturnJob(make_menu);
		b->addObject(see);
	}
}

void EntitySequenceView::distanceButton()
{
	ImageButton *b = addButton("assets/images/ruler.png", Ruler,
	                           0.03, "measuring distances");
	b->setReturnJob([this]() { angleButton(); });
}

void EntitySequenceView::setup()
{
	SequenceView::setup();

	distanceButton();
}

void EntitySequenceView::handleResidue(Button *button, Residue *r)
{
	if (_mode != Defining)
	{
		PickAtomFromSequence::handleResidue(button, r);
	}
	else
	{
		_curr = r;
		_candidate = "";
		confirmAtom();
	}
}

void EntitySequenceView::handleAtomName(std::string name)
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

void EntitySequenceView::confirmAtom()
{
	if (_stage == Nothing)
	{
		_first = _candidate;
		_aRes = _curr;
		_stage = ChosenFirst;
		if (_mode == Defining)
		{
			refresh();
		}
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
		else if (_mode == Defining)
		{
			defineRegion();
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

void EntitySequenceView::prepareAngles()
{
	Atom3DPosition one(_aRes, _first);
	Atom3DPosition two(_bRes, _second);
	Atom3DPosition three(_cRes, _third);

	PolymerEntity::FindAtom get_first = search_models_any_instance(_entity, one);
	PolymerEntity::FindAtom get_second = search_models_same_instance(_entity, two);
	PolymerEntity::FindAtom get_third = search_models_same_instance(_entity, three);

	std::string header;
	header += _aRes->id().as_string() + _first;
	header += " through " + _bRes->id().as_string() + _second;
	header += " to " + _cRes->id().as_string() + _third;

	Metadata *md = _entity->funcBetweenAtoms({get_first, get_second, get_third},
	                                         header, &angle_between, closest_to(0));
	queueMetadataForShow(md);
}

void EntitySequenceView::queueMetadataForShow(Metadata *md)
{
	auto display_metadata = [this, md]()
	{
		MetadataView *mv = new MetadataView(this, md);
		mv->show();
		_stage = Nothing;
	};
	
	addMainThreadJob(display_metadata);
}

void EntitySequenceView::prepareDistances()
{
	Atom3DPosition one(_aRes, _first);
	Atom3DPosition two(_bRes, _second);
	
	PolymerEntity::FindAtom get_first = search_models_any_instance(_entity, one);
	PolymerEntity::FindAtom get_second = search_models_same_instance(_entity, two);

	std::string header;
	header += _aRes->id().as_string() + _first;
	header += " to " + _bRes->id().as_string() + _second;

	Metadata *md = _entity->funcBetweenAtoms({get_first, get_second}, header,
	                                         &distance_between, closest_to(0));
	queueMetadataForShow(md);
}

void EntitySequenceView::stop()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}
}

void EntitySequenceView::calculateDistance()
{
	std::string title = "Calculating distances...";
	VagWindow::window()->requestProgressBar(_entity->modelCount(), title);
	stop();

	_worker = new std::thread(&EntitySequenceView::prepareDistances, this);
}

void EntitySequenceView::calculateAngle()
{
	std::string title = "Calculating angles...";
	VagWindow::window()->requestProgressBar(_entity->modelCount(), title);
	stop();

	_worker = new std::thread(&EntitySequenceView::prepareAngles, this);
}


void EntitySequenceView::buttonPressed(std::string tag, Button *button)
{
	std::string atom = Button::tagEnd(tag, "atomname_");
	
	if (atom.length())
	{
		handleAtomName(atom);
		return;
	}
	
	std::string yes = Button::tagEnd(tag, "yes_");
	
	if (yes == "confirm_atom")
	{
		confirmAtom();
		return;
	}
	
	if (tag == "name_region")
	{
		TextEntry *entry = static_cast<TextEntry *>(button);
		std::string name = entry->scratch();
		makeRegion(name, _aRes, _bRes);
		definingButton();
	}

	SequenceView::buttonPressed(tag, button);
}

void EntitySequenceView::setEntity(Entity *ent)
{
	_entity = Environment::entityManager()->entity(ent->name());

}

void EntitySequenceView::addExtras(TextButton *t, Residue *r) 
{
	PickAtomFromSequence::addExtras(t, r);
	
	auto hover_job = [this, r]()
	{
		if (_stage == ChosenFirst && _mode == Defining)
		{
			_bRes = r;
			refresh();
		}
	};

	t->setHoverJob(hover_job);

	if (_mode == Defining && _aRes && _bRes && 
	    r->id() >= _aRes->id() && r->id() <= _bRes->id())
	{
		t->resize(1.2);
	}
}

void EntitySequenceView::defineRegion()
{
	std::string str = "Name for new region, residues " + 
	_aRes->id().str() + "-" + _bRes->id().str();

	AskForText *aft = new AskForText(this, str, "name_region",
	                                 this, TextEntry::Id);
	setModal(aft);
}

void EntitySequenceView::makeRegion(const std::string &name,
                                    Residue *a, Residue *b)
{
	Region reg;
	reg.setEntityId(_entity->name());
	reg.setId(name);
	reg.setRange(a->id(), b->id());

	try
	{
		_entity->regionManager().insertIfUnique(reg);
		definingButton();
	}
	catch (const std::runtime_error &err)
	{
		BadChoice *bc = new BadChoice(this, err.what());
		setModal(bc);
	}
}
