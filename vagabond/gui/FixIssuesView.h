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

#ifndef __vagabond__FixIssuesView__
#define __vagabond__FixIssuesView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/FixIssues.h>

class Entity;
class FixIssues;
class Polymer;
class ChoiceText;
class TickBoxes;

class FixIssuesView : public Scene
{
public:
	FixIssuesView(Scene *prev, Entity *ent);
	~FixIssuesView();

	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button);
private:
	FixIssues::Options options();
	void stop();

	Entity *_entity = nullptr;
	Polymer *_molecule = nullptr;

	FixIssues *_fixer = nullptr;
	TickBoxes *_tickboxes = nullptr;
	std::thread *_worker = nullptr;
	
	std::vector<ChoiceText *> _options;
};

#endif
