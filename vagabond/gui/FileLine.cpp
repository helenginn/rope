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

#include "FileView.h"
#include "FileLine.h"
#include <vagabond/gui/elements/Image.h>
#include <vagabond/utils/FileReader.h>

#include <vagabond/gui/elements/TextButton.h>

#include <iostream>

FileLine::FileLine(FileView *view, std::string filename, std::string display)
{
	_view = view;
	_filename = filename;
	_display = display;
	if (_display == "")
	{
		_display = _filename;
	}
	setName("FileLine");
	
	queryFile();
	setup();
}

FileLine::~FileLine()
{

}

void FileLine::queryFile()
{
	_type = File::typeUnknown(_filename);
}

void FileLine::setup()
{
	TextButton *button = new TextButton(_display, _view);
	button->setReturnTag("file_" + _filename);
	button->resize(0.9);
	button->setLeft(0.0, 0);
	
	if (is_directory(_filename))
	{
		button->setInert(true);
		button->setAlpha(-0.4);
	}

	addObject(button);
	
	if (_type & File::UnitCell)
	{
		Image *image = new Image("assets/images/unit_cell.png");
		image->resize(0.08);
		image->addAltTag("has unit cell");
		image->setCentre(0.40, 0);
		addObject(image);
	}
	
	if (_type & File::Reflections)
	{
		Image *image = new Image("assets/images/reflections.png");
		image->resize(0.08);
		image->addAltTag("has reflections");
		image->setCentre(0.45, 0);
		addObject(image);
	}
	
	if (_type & File::CompAtoms)
	{
		Image *image = new Image("assets/images/some_atoms.png");
		image->resize(0.08);
		image->addAltTag("has ligand atoms");
		image->setCentre(0.50, 0);
		
		addObject(image);
	}
	
	if (_type & File::Geometry)
	{
		Image *image = new Image("assets/images/protractor.png");
		image->resize(0.08);
		image->addAltTag("has geometry info");
		image->setCentre(0.55, 0);
		addObject(image);
	}
	
	if (_type & File::MacroAtoms)
	{
		Image *image = new Image("assets/images/protein.png");
		image->resize(0.08);
		image->setCentre(0.6, 0);
		image->addAltTag("has macromolecule atoms");
		addObject(image);
	}
	
	if (_type & File::Meta)
	{
		Image *image = new Image("assets/images/metadata.png");
		image->resize(0.06);
		image->setCentre(0.65, 0);
		image->addAltTag("metadata information");
		addObject(image);
	}

	if (_type & File::Sequence)
	{
		Image *image = new Image("assets/images/sequence.png");
		image->resize(0.06);
		image->setCentre(0.7, 0);
		image->addAltTag("has sequence info");
		addObject(image);
	}
}
