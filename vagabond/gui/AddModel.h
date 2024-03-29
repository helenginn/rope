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

#ifndef __vagabond__AddModel__
#define __vagabond__AddModel__

#include <vagabond/gui/elements/TextButton.h>
#include "FileView.h"
#include "AddObject.h"
#include <vagabond/core/Model.h>

class TextEntry;

class AddModel : public AddObject<Model>, public Responder<FileView>
{
public:
	AddModel(Scene *prev, Model *chosen);
	AddModel(Scene *prev);
	~AddModel();

	virtual void setup();
	
	const Model &model() const
	{
		return _obj;
	}

	void refreshInfo();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual void sendObject(std::string filename, void *object);
private:
	void fileTextOrChoose(std::string &file, std::string other = "Choose...");
	TextButton *_initialFile = nullptr;
	TextButton *_initialData = nullptr;
	TextEntry *_name = nullptr;
	std::string _lastTag;
};

#endif
