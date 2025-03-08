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

#ifndef __vagabond__FileLine__
#define __vagabond__FileLine__

#include <vagabond/utils/os.h>
#ifdef OS_WINDOWS
#define NOMINMAX
#endif
#include <vagabond/core/files/CifFile.h>
#include <vagabond/gui/elements/Box.h>

class FileView;

class FileLine : public Box
{
public:
	FileLine(FileView *view,std::string filename, std::string display = "");
	virtual ~FileLine();

private:
	void setup();
	void queryFile();

	FileView *_view;
	std::string _filename;
	std::string _display;
	CifFile::Type _type;
};

#endif
