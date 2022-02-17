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

#include <iostream>
#include "FileView.h"
#include "FileLine.h"
#include "TextButton.h"
#include "Image.h"

FileLine::FileLine(FileView *view, std::string filename)
{
	_view = view;
	_filename = filename;
	setName("FileLine");
	
	queryFile();
	setup();
}

FileLine::~FileLine()
{
	deleteObjects();
}

void FileLine::queryFile()
{
	CifFile cf(_filename);
	
	_type = cf.cursoryLook();
}

void FileLine::setup()
{
	TextButton *button = new TextButton(_filename, _view);
	button->setReturnTag("file_" + _filename);
	button->resize(0.9);
	button->setLeft(0.2, 0);
	addObject(button);
	
	if (_type & CifFile::CompAtoms)
	{
		Image *prot = new Image("assets/images/some_atoms.png");
		prot->resize(0.08);
		prot->setCentre(0.70, 0);
		addObject(prot);
	}
	
	if (_type & CifFile::Geometry)
	{
		Image *prot = new Image("assets/images/protractor.png");
		prot->resize(0.08);
		prot->setCentre(0.75, 0);
		addObject(prot);
	}
	
	if (_type & CifFile::MacroAtoms)
	{
		Image *macro = new Image("assets/images/protein.png");
		macro->resize(0.08);
		macro->setCentre(0.8, 0);
		addObject(macro);
	}

}
