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

#include "CommunicationChoice.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskMultipleChoice.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/core/protonic/Clique.h>

CommunicationChoice::CommunicationChoice(Scene *prev, Clique *clique)
: ListView(prev), _clique(clique)
{
	_candidates = clique->nonWaterProbes().toVector();

}

void CommunicationChoice::setup()
{
	addTitle("Choose communication groups");

	ListView::setup();
}

void CommunicationChoice::refresh()
{
	ListView::refresh();
	
	auto to_descs = [this]()
	{
		OpSet<std::string> descs;
		for (const int &sel : _selected)
		{
			descs += _candidates[sel]->desc();
		}
		return descs;
	};
	
	auto assign_to = [this, to_descs](const std::string &name)
	{
		return [this, to_descs, name]()
		{
			_clique->addCommunicationPoints(name, to_descs());
			_selected = {};
			refresh();
		};
	};
	
	auto assign = [this, assign_to, to_descs]()
	{
		AskMultipleChoice *amc = new AskMultipleChoice(this, "Assign to group", 
		                                               true);
		OpSet<std::string> names = _clique->allCommsNames();

		for (const std::string &name : names)
		{
			amc->addChoice(name, assign_to(name));
		}
		
		auto make_new = [this, to_descs](std::string new_name)
		{
			_clique->addCommunicationPoints(new_name, to_descs());
			_selected = {};
			refresh();
		};
		
		auto ask_for_new = [this, make_new]()
		{
			AskForText *aft = new AskForText(this, "New group name:", "", this);
			aft->setReturnJob(make_new);
			setModal(aft);
		};
		
		amc->addChoice("New group", ask_for_new);
		
		setModal(amc);
	};
	
	auto deassign = [this, to_descs]()
	{
		_clique->removeCommunicationPoints(to_descs());
		_selected = {};
		refresh();
	};

	if (_selected.size())
	{
		TextButton *tb = new TextButton("assign to group", this);
		tb->setCentre(0.3, 0.2);
		tb->setReturnJob(assign);
		tb->resize(0.6);
		addTempObject(tb);
		
		for (const int &sel : _selected)
		{
			if (_clique->groupOfNode(_candidates[sel]->desc()).length() > 0)
			{
				TextButton *tb = new TextButton("clear assignment", this);
				tb->setCentre(0.7, 0.2);
				tb->setReturnJob(deassign);
				tb->resize(0.6);
				addTempObject(tb);

				break;
			}
		}
	}
}

size_t CommunicationChoice::lineCount()
{
	return _candidates.size();
}

Renderable *CommunicationChoice::getLine(int i)
{
	Box *box = new Box();
	
	auto select_candidate = [this, i]()
	{
		if (_selected.count(i))
		{
			_selected.erase(i);
		}
		else
		{
			_selected.insert(i);
		}

		refresh();
	};
	
	{
		std::string str = (_selected.count(i) ? "* " : "");
		str += _candidates[i]->desc();
		TextButton *tb = new TextButton(str, this);
		tb->setLeft(0., 0.);
		tb->resize(0.6);
		tb->setReturnJob(select_candidate);
		box->addObject(tb);
	}
	
	{
		std::string desc = _candidates[i]->desc();
		Text *t = new Text(_clique->groupOfNode(desc));
		t->setRight(0.6, 0.);
		t->resize(0.6);
		box->addObject(t);
	}

	return box;
}
