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

#ifndef __vagabond__SearchPDB__
#define __vagabond__SearchPDB__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/Fetcher.h>

class Entity;
class TextEntry;
class Text;

class SearchPDB : public Scene, public Fetcher
{
public:
	SearchPDB(Scene *prev, Entity *ent);

	virtual void setup();
	virtual void refresh();
	virtual void doThings();
	virtual void buttonPressed(std::string tag, Button *button);

protected:
	std::string prepareQuery();
	virtual void processResult(std::string result);
	virtual  void handleError();
	virtual std::string toURL(std::string query);
private:
	Entity *_entity;

	int _rows = 500;
	int _cutoff = 98;
	TextButton *_cutoffText = nullptr;
	TextEntry *_maxRowsText = nullptr;
};

#endif
