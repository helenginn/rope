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

#ifndef __vagabond__ChooseEntity__
#define __vagabond__ChooseEntity__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/gui/ChainAssignment.h>

class SequenceComparison;
class Chain;
class Model;

class ChooseEntity : public ListView
{
public:
	ChooseEntity(Scene *prev, Model &model, Chain *chain);
	
	void setCaller(ChainAssignment *caller)
	{
		_caller = caller;
	}

	virtual void setup();

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	void setEntity(std::string name);
	void findAlignments();
private:
	PolymerEntityManager *_manager;
	Chain *_chain = nullptr;
	ChainAssignment *_caller = nullptr;

	Model &_model;
	std::map<Entity *, SequenceComparison *> _entity2Alignment;
};

#endif
