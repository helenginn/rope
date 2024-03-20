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

#include "PickAtomFromSequence.h"
#include "PolymerEntity.h"
#include "CompareEntities.h"
#include "ChooseHeader.h"
#include "MetadataView.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/AtomRecall.h>

CompareEntities::CompareEntities(Scene *scene) : Scene(scene)
{

}

void CompareEntities::setup()
{
	addTitle("Entity metrics");

	{
		Text *measure = new Text("Measure");
		measure->setLeft(0.15, 0.22);
		addObject(measure);
	}
	
	{
		TickBoxes *tb = new TickBoxes(this, this);
		tb->addOption("distance");
		tb->addOption("angle");
		tb->arrange(0.4, 0.22, 0.9, 0.5);
		addObject(tb);
	}
}

std::set<std::string> CompareEntities::acceptableEntityList()
{
	std::set<std::string> ids;
	EntityManager *em = EntityManager::manager();
	std::vector<Entity *> entities = em->entities();

	for (Entity *entity : entities)
	{
		ids.insert(entity->name());
		if (_entries[0].id.length() && _entries[0].id == entity->name())
		{
			ids.insert("different " + entity->name());
		}
	}
	
	return ids;
}

bool CompareEntities::proofAndShowAtom(TextButton *button, 
                                       CompareEntities::Entry &entry)
{
	button->setText("choose...");
	
	if (entry.master == nullptr || entry.atomName.length() == 0)
	{
		return false;
	}
	
	std::string text = entry.atom_desc();
	button->setText(text);
	
	return true;
}

bool CompareEntities::proofAndShowEntity(TextButton *button,
                                         std::string &id, int idx)
{
	button->setText("choose...");
	
	if (id.length() == 0)
	{
		return false;
	}

	std::set<std::string> acceptable = acceptableEntityList();
	for (const std::string &acc : acceptable)
	{
		std::cout << acc << ", ";
	}
	std::cout << std::endl;
	
	if (Button::tagEnd(id, "different ").length() > 0 && idx == 0)
	{
		id = "";
		return false;
	}
	
	if (acceptable.count(id))
	{
		button->setText(id);
		return true;
	}

	id = "";
	return false;
}

bool CompareEntities::showEntityOptions()
{
	int max = requiredCount();
	std::cout << "Max: " << max << std::endl;
	bool full_house = true;

	float top = 0.35;
	for (size_t i = 0; i < max; i++)
	{
		Text *text = new Text("Entity " + std::to_string(i + 1));
		text->setLeft(0.15, top);
		addTempObject(text);
		
		TextButton *choose = new TextButton("choose...", this);
		choose->setReturnTag("choose_" + std::to_string(i));
		choose->setLeft(0.35, top);
		bool got_entity = proofAndShowEntity(choose, _entries[i].id, i);
		full_house &= got_entity;
		addTempObject(choose);
		
		if (got_entity)
		{
			Text *text = new Text("Atom:");
			text->setLeft(0.6, top);
			addTempObject(text);
			
			TextButton *choose = new TextButton("choose...", this);
			choose->setReturnTag("atom_" + std::to_string(i));
			choose->setRight(0.9, top);
			bool got_atom = proofAndShowAtom(choose, _entries[i]);
			full_house &= got_atom;
			addTempObject(choose);
		}

		top += 0.06;
	}
	
	return full_house;
}

size_t CompareEntities::requiredCount()
{
	switch (_measureMode)
	{
		case None: return 0;
		case Distance: return 2;
		case Angle: return 3;
		default: return 0;
	}
}

bool CompareEntities::allEntriesIdentical()
{
	int max = requiredCount();
	for (size_t i = 1; i < max; i++)
	{
		if (_entries[i].id != _entries[0].id)
		{
			return false;
		}
	}

	return true;
}

void CompareEntities::showScoreOptions()
{
	if (allEntriesIdentical())
	{
		Text *t = new Text("These settings should produce no duplicate metrics.");
		t->setLeft(0.15, .6);
		addTempObject(t);

		TextButton *ok = new TextButton("Calculate", this);
		ok->setRight(0.9, 0.9);
		ok->setReturnTag("calculate");
		addTempObject(ok);

		return;
	}

	Text *t = new Text("In the event of multiple results, how shall RoPE choose?");
	t->setLeft(0.15, .6);
	addTempObject(t);

	std::cout << "Adding score options here" << std::endl;
	TickBoxes *tb = new TickBoxes(this, this);
	tb->addOption("smallest value", "smallest");
	tb->addOption("closest to:", "closest");
	tb->setVertical(true);
	tb->arrange(0.4, 0.67, 0.9, 0.85);
	addTempObject(tb);
	
	std::string chosen = (_target < 0 ? "smallest" : "closest");
	tb->tick(chosen);
	
	std::string str = (_target < 0 ? "enter..." : f_to_str(_target, 2));
	TextEntry *te = new TextEntry(str, this, this);
	te->setValidationType(TextEntry::Numeric);
	te->setReturnTag("target");
	te->setRight(0.65, 0.76);
	addTempObject(te);

	TextButton *ok = new TextButton("Calculate", this);
	ok->setRight(0.9, 0.9);
	ok->setReturnTag("calculate");
	addTempObject(ok);
}

void CompareEntities::refresh()
{
	deleteTemps();

	bool chosen_all_atoms = showEntityOptions();
	
	if (chosen_all_atoms)
	{
		showScoreOptions();
	}
}

void CompareEntities::chooseAtom(int idx)
{
	_active = idx;
	_chooseMode = ChooseAtom;
	
	EntityManager *em = EntityManager::manager();
	Entity *ent = em->entity(_entries[idx].raw_id);
	
	if (!ent)
	{
		_entries[idx].id = "";
		refresh();
		return;
	}

	PickAtomFromSequence *pick = new PickAtomFromSequence(this, ent->sequence());
	pick->setResponder(this);
	pick->show();
}

void CompareEntities::chooseHeaders(int idx)
{
	_active = idx;
	_chooseMode = ChooseEntity;
	std::set<std::string> acceptable = acceptableEntityList();

	ChooseHeader *ch = new ChooseHeader(this, true);
	ch->setHeaders(acceptable);
	ch->setTitle("Choose entity " + std::to_string(idx));
	ch->setResponder(this);
	ch->show();
}

void CompareEntities::sendObject(std::string tag, void *object)
{
	if (_chooseMode == ChooseEntity &&
	    (_entries[_active].id != tag && _entries[_active].id != tag))
	{
		_entries[_active] = Entry{};
		_entries[_active].id = tag;
		_entries[_active].raw_id = tag;

		std::string name = Button::tagEnd(tag, "different ");
		if (name.length())
		{
			_entries[_active].raw_id = name;
		}
	}
	else if (_chooseMode == ChooseAtom)
	{
		typedef std::pair<Residue *, std::string> AtomRes;
		AtomRes ar = *static_cast<AtomRes *>(object);
		_entries[_active].master = ar.first;
		_entries[_active].atomName = ar.second;
	}

	_active = -1;
	refresh();
}

void CompareEntities::buttonPressed(std::string tag, Button *button)
{
	std::cout << "tag: " << tag << std::endl;

	if (tag == "distance" || tag == "angle")
	{
		TickBoxes *tb = static_cast<TickBoxes *>(button);
		
		if (!tb->isTicked(tag))
		{
			_measureMode = None;
		}
		else if (tag == "distance")
		{
			_measureMode = Distance;
		}
		else if (tag == "angle")
		{
			_measureMode = Angle;
		}
	}
	
	{
		std::string num = Button::tagEnd(tag, "choose_");

		if (num.length())
		{
			int idx = atoi(num.c_str());
			chooseHeaders(idx);
		}
	}

	{
		std::string num = Button::tagEnd(tag, "atom_");

		if (num.length())
		{
			int idx = atoi(num.c_str());
			chooseAtom(idx);
		}
	}
	
	if (tag == "smallest")
	{
		_target = -1;
	}
	else if (tag == "closest")
	{
		_target = 10;
	}
	
	if (tag == "target")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string words = te->scratch();
		float target = atof(words.c_str());
		_target = target;
	}
	
	if (tag == "calculate")
	{
		calculate();
	}

	refresh();
	Scene::buttonPressed(tag, button);
}

std::string CompareEntities::makeHeader()
{
	std::string header;
	header += _entries[0].atom_desc() + " in " + _entries[0].raw_id;
	header += (_measureMode == Distance ? " to " : " through ");
	header += _entries[1].atom_desc() + " in " + _entries[1].raw_id;

	if (_measureMode == Angle)
	{
		header += " to ";
		header += _entries[2].atom_desc() + " in " + _entries[2].raw_id;
	}

	return header;
}

std::string CompareEntities::Entry::atom_desc()
{
	return master->desc() + ":" + atomName;
}

CompareEntities::Entry::FindAtom CompareEntities::Entry::as_find_function()
{
	Atom3DPosition one(master, atomName);
	CompareEntities::Entry::FindAtom find;
	Entity *entity = EntityManager::manager()->entity(raw_id);

	if (id == raw_id)
	{
		find = search_models_any_instance(entity, one);
	}
	else
	{
		find = search_models_different_instance(entity, one);
	}

	return find;
}

void CompareEntities::calculate()
{
	int max = requiredCount();
	std::vector<CompareEntities::Entry::FindAtom> finds;

	for (size_t i = 0; i < max; i++)
	{
		CompareEntities::Entry::FindAtom find = _entries[i].as_find_function();
		finds.push_back(find);
	}
	
	std::string header = makeHeader();
	auto compare = closest_to(_target);

	auto measure = (_measureMode == Distance ? 
	                &distance_between : &angle_between);
	
	Entity *entity = EntityManager::manager()->entity(_entries[0].raw_id);
	
	Metadata *md = entity->funcBetweenAtoms(finds, header, measure, compare);

	MetadataView *mv = new MetadataView(this, md);
	mv->show();
}
