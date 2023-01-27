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

#ifndef __vagabond__EntityFromSequence__
#define __vagabond__EntityFromSequence__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/Fetcher.h>
#include "FileView.h"
#include <thread>

class EntityFromSequence : public Scene, public Fetcher,
public Responder<FileView>
{
public:
	EntityFromSequence(Scene *prev);
	~EntityFromSequence();

	virtual void setup();
	virtual void render();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual std::string toURL(std::string query);
	virtual void processResult(std::string seq);
	virtual void handleError();
	virtual void fileChosen(std::string filename);
private:
	void makePeptide(std::string text);

	std::string _url;

};

#endif
