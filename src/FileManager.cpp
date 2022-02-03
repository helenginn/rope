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

#include "FileManager.h"
#include "FileReader.h"
#include "FileView.h"

FileManager::FileManager()
{
	_view = nullptr;
	_list.push_back("assets/geometry/GLY.cif");
	_list.push_back("assets/geometry/ASP.cif");
	_list.push_back("assets/geometry/ATP.cif");
	_list.push_back("assets/geometry/L86.cif");
	_list.push_back("assets/geometry/TYR.cif");
	_list.push_back("assets/geometry/CHX.cif");
	_list.push_back("assets/examples/2ybh.cif");
}

void FileManager::acceptFile(std::string filename, bool force)
{
	if (file_exists(filename) || force)
	{
		_list.push_back(filename);
	}
	
	if (_view != nullptr)
	{
		_view->refreshFiles();
	}
}

