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

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/TextButton.h>
#include "FileView.h"
#include <vagabond/core/Model.h>

class TextEntry;

class AddModel : public virtual Scene, public FileViewResponder
{
public:
	AddModel(Scene *prev, Model *chosen = nullptr);
	~AddModel();

	void setup();
	
	const Model &model() const
	{
		return _m;
	}

	void refreshInfo();
	virtual void buttonPressed(std::string tag, Button *button = NULL);
	virtual void fileChosen(std::string filename);
private:
	void fileTextOrChoose(std::string &file, std::string other = "Choose...");
	TextButton *_initialFile = nullptr;
	TextEntry *_name = nullptr;
	Model _m;
	std::string _lastTag;

	bool _existing = false;
};

#endif
