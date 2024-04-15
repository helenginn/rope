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

#ifndef __vagabond__MakeNewPaths__
#define __vagabond__MakeNewPaths__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>

class Path;
class Entity;
class Instance;
class TextButton;
class ChooseHeader;

class MakeNewPaths : public Scene, public Responder<ChooseHeader>
{
public:
	MakeNewPaths(Scene *prev, Entity *entity);
	
	void setBlueprint(Path *blueprint)
	{
		_blueprint = blueprint;
	}
	
	void setPriorStartEnd(Instance *start, Instance *end);

	virtual void setup();
	virtual void refresh();
	virtual void buttonPressed(std::string tag, Button *button);
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	void checkAndPrepare();
	void prepare();
	void setStructure(const std::string &str);
	void getStructure(bool from);
	bool _from = true;

	void addFloatEntry(float top, const std::string &desc, 
	                   float *target);
	void addIntEntry(float top, const std::string &desc, 
	                   int *target);

	Entity *const _entity;
	TextButton *_fromButton = nullptr;
	TextButton *_toButton = nullptr;
	Path *_blueprint = nullptr;

	float _maxMomentumDistance = 8.f;
	float _maxClashDistance = 15.f;
	int _maxFlipTrial = 0;
	bool _restart = true;
	std::string _fromId;
	std::string _toId;
};

#endif
