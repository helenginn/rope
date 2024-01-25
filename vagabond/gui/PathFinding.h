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
#include <vagabond/core/TorsionData.h>
#include <vagabond/core/Responder.h>

class SpecificNetwork;
class ChooseHeader;
class ObjectData;
class Instance;
class Entity;
class Rule;
class Text;

class PathFinding : public Scene, public Responder<ChooseHeader>
{
public:
	PathFinding(Scene *prev, Entity *ent);
	~PathFinding();

	virtual void refresh();
	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button);
protected:
	void sendObject(std::string tag, void *object);
private:
	std::vector<Instance *> activeInstances();
	void loadSpace(std::string filename);
	void setStructureIfValid(const std::string &str);

	void getStructure();
	void deleteRule();
	void makeRule();
	void findJson();
	void start();

	Entity *_entity = nullptr;
	Text *_refText = nullptr;
	Instance *_reference = nullptr;
	TorsionData _all;
	Rule *_rule = nullptr;
	SpecificNetwork *_prior = nullptr;
};

#endif
