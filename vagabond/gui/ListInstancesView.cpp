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

#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Model.h>
#include "ChooseHeader.h"
#include "ListInstancesView.h"
#include "PathInterfaceSetupView.h"
#include <algorithm>

ListInstancesView::ListInstancesView(Scene *prev) : ListView(prev)
{

}

void ListInstancesView::setup()
{
	addTitle("Include instances");
	applyDefaultCheatSheet();
	ListView::setup();
}

void ListInstancesView::applyDefaultCheatSheet()
{
	if (_defaultCheatSheetApplied)
	{
		return;
	}

	_defaultCheatSheetApplied = true;
	std::string chosen = automaticCheatSheet();
	if (chosen.length())
	{
		applyCheatSheet(chosen, false);
	}
}

static bool string_contains(const std::string &haystack,
                            const std::string &needle)
{
	return needle.length() && haystack.find(needle) != std::string::npos;
}

bool ListInstancesView::cheatSheetMatchesModels(const std::string &filename)
{
	if (!_fromModel || !_toModel || !file_exists(filename))
	{
		return false;
	}

	std::string contents = get_file_contents(filename);
	std::vector<std::string> lines = split(contents, '\n');

	for (const std::string &line : lines)
	{
		std::vector<std::string> bits = split(line, ' ');
		if (bits.size() < 2) continue;

		bool from_match = string_contains(bits[0], _fromModel->name());
		bool to_match = string_contains(bits[1], _toModel->name());

		if (from_match && to_match)
		{
			return true;
		}
	}

	return false;
}

std::string ListInstancesView::automaticCheatSheet()
{
	std::vector<std::string> matches = glob_pattern("cheat_sheet*.txt");
	std::sort(matches.begin(), matches.end());

	for (const std::string &match : matches)
	{
		if (cheatSheetMatchesModels(match))
		{
			std::cout << "Automatically applying " << match << std::endl;
			return match;
		}
	}

	return "";
}

std::string ListInstancesView::chosenCheatSheetLabel()
{
	const std::string prefix = "cheat_sheet_";
	const std::string suffix = ".txt";
	const std::string &name = _chosenCheatSheet;

	if (name == "cheat_sheet.txt")
	{
		return "\"Jinx!\"";
	}

	if (name.rfind(prefix, 0) != 0 || name.length() <= prefix.length())
	{
		return "";
	}

	if (name.length() >= suffix.length() &&
	    name.substr(name.length() - suffix.length()) == suffix)
	{
		return name.substr(prefix.length(),
		                   name.length() - prefix.length() - suffix.length());
	}

	return name.substr(prefix.length());
}

bool ListInstancesView::applyCheatSheet(const std::string &filename,
                                        bool refresh_after)
{
	if (!file_exists(filename))
	{
		return false;
	}

	for (const std::string &from : _fromList)
	{
		_fromTo[from] = {false, ""};
	}

	std::string contents = get_file_contents(filename);
	std::vector<std::string> lines = split(contents, '\n');
	bool applied = false;

	for (const std::string &line : lines)
	{
		std::vector<std::string> bits = split(line, ' ');
		if (bits.size() < 2) continue;

		if (std::find(_fromList.begin(), _fromList.end(), bits[0]) ==
		    _fromList.end()) continue;
		if (std::find(_toList.begin(), _toList.end(), bits[1]) ==
		    _toList.end()) continue;

		std::cout << bits[0] << " = " << bits[1] << std::endl;
		_fromTo[bits[0]] = {true, bits[1]};
		applied = true;
	}

	if (applied)
	{
		_chosenCheatSheet = getFilename(filename);
	}
	else
	{
		_chosenCheatSheet = "";
	}

	if (refresh_after)
	{
		refresh();
	}

	return applied;
}

std::vector<std::string> ListInstancesView::cheatSheetFiles()
{
	std::vector<std::string> files;
	if (file_exists("cheat_sheet.txt"))
	{
		files.push_back("cheat_sheet.txt");
	}

	std::vector<std::string> matches = glob_pattern("cheat_sheet_*.txt");
	std::sort(matches.begin(), matches.end());
	for (const std::string &match : matches)
	{
		if (std::find(files.begin(), files.end(), match) == files.end())
		{
			files.push_back(match);
		}
	}

	return files;
}

void ListInstancesView::chooseCheatSheet(TextButton *button)
{
	std::vector<std::string> files = cheatSheetFiles();
	if (files.empty())
	{
		setInformation("No cheat_sheet.txt or cheat_sheet_*.txt files found.");
		return;
	}

	Menu *m = new Menu(this);
	for (const std::string &file : files)
	{
		m->addOption(getFilename(file), [this, file]()
		{
			applyCheatSheet(file);
		});
	}
	m->setup(button);
	setModal(m);
}

void ListInstancesView::refresh()
{
	deleteTemps();
	ListView::refresh();

	if (!cheatSheetFiles().empty())
	{
		TextButton *t = new TextButton("Cheat sheet(s)", this);
		t->setRight(0.88, 0.18);
		t->setReturnJob([this, t]() { chooseCheatSheet(t); });
		addTempObject(t);

		std::string label = chosenCheatSheetLabel();
		if (label.length())
		{
			Text *chosen_text = new Text(label);
			chosen_text->setRight(0.88, 0.24);
			addTempObject(chosen_text);
		}
	}

	size_t chosen = 0;
	for (auto it = _fromTo.begin(); it != _fromTo.end(); it++)
	{
		auto &pair = it->second;
		if (pair.active && pair.id.length() > 0)
		{
			chosen++;
		}
	}

	std::string str = std::to_string(chosen) + " pair";
	str += chosen == 1 ? "" : "s";
	Text *info = new Text(str);
	info->setRight(0.8, 0.82);
	addTempObject(info);

	if (chosen > 1)
	{
		TextButton *t = new TextButton("Find path(s)", this);
		t->setRight(0.8, 0.88);
		t->setReturnJob
		([this]()
		 {
			std::map<std::string, std::string> simple_map;
			for (auto it = _fromTo.begin(); it != _fromTo.end(); it++)
			{
				auto &pair = it->second;
				if (pair.active && pair.id.length() > 0)
				{
					simple_map[it->first] = pair.id;
				}
			}

			PathInterfaceSetupView::preparePath(this, _fromModel, _toModel,
			                                    simple_map);
		 });

		addTempObject(t);
	}
}

size_t ListInstancesView::lineCount()
{
	return _fromList.size();
}

Renderable *ListInstancesView::getLine(int i)
{
	Box *b = new Box();
	{
		TickBoxes *tb = new TickBoxes(this, this);
		std::string text = _fromList[i];
		bool ticked = _fromTo[text].active;
		tb->addOption(text, text, ticked);
		tb->arrange(0, 0, 0.5, 0.1);
		if (_toList.size())
		{
			tb->setReturnJob
			([this, tb, text]()
			 {
				bool ticked = tb->isTicked(text);
				_fromTo[text].active = ticked;
				
				refresh();
			 });
		}

		b->addObject(tb);
		
		if (_fromTo[text].active)
		{
			Image *image = new Image("assets/images/axis.png");
			glm::mat3x3 rot = glm::mat3x3(glm::rotate(glm::mat4(1.), 
			                                          (float)deg2rad(-90),
			                                          glm::vec3(0., 0., -1.)));
			image->rotateRoundCentre(rot);
			image->rescale(0.012, 0.04);
			image->setCentre(0.3, 0);
			b->addObject(image);

			std::string inviteText = _fromTo[text].id;
			if (inviteText.length() == 0) { inviteText = "Choose..."; }
			TextButton *rhs = new TextButton(inviteText, this);
			rhs->setRight(0.6, 0);
			rhs->setReturnJob([this, text]()
			{
				_selected = text;

				ChooseHeader *ch = new ChooseHeader(this, true);
				ch->setResponder(this);
				ch->setHeaders(_toList);
				ch->setTitle("Set target instance from \"to\" model");
				ch->show();
			
			});
			b->addObject(rhs);
		}
	}

	return b;
}

void ListInstancesView::sendObject(std::string tag, void *object)
{
	_fromTo[_selected].id = tag;
	_fromTo[_selected].active = true;
	std::cout << _selected << " to " << tag << std::endl;
	
	refresh();
}

void ListInstancesView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		sendResponse("instance_map", &_fromTo);
	}
	
	ListView::buttonPressed(tag, button);
}
