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

#ifndef __vagabond__PathFinding__
#define __vagabond__PathFinding__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/MetadataGroup.h>

class SpecificNetwork;
class ObjectGroup;
class Entity;
class Rule;

class PathFinding : public Scene
{
public:
	PathFinding(Scene *prev, Entity *ent);
	~PathFinding();

	virtual void refresh();
	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button);
private:
	void deleteRule();
	void makeRule();
	void findJson();
	void loadSpace(std::string filename);
	void start();

	Entity *_entity = nullptr;
	MetadataGroup _all;
	Rule *_rule = nullptr;
	SpecificNetwork *_prior = nullptr;
};

#endif
