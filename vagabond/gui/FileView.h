// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__FileView__
#define __vagabond__FileView__

#include <iostream>
#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/File.h>
#include <vagabond/core/Responder.h>

class FileManager;

/*
class FileViewResponder : public virtual Scene
{
public:
	FileViewResponder(Scene *prev) {}
	virtual ~FileViewResponder() {};
	virtual void fileChosen(std::string filename) = 0;
};
*/

class FileView : public ListView, public Responder<FileManager>,
public HasResponder<Responder<FileView> >
{
public:
	FileView(Scene *prev);
	FileView(Scene *prev, Responder<FileView> *resp, bool choose);
	
	void filterForTypes(File::Type type);
	
	virtual ~FileView();
	virtual void setup();
	
	virtual void respond();
	void handleFileWithoutChoice(std::string filename);
	void returnToResponder(std::string filename);

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void loadFilesFrom(int start, int num);
	FileManager *_manager;
};

#endif
