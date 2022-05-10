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

#include <vagabond/utils/FileReader.h>
#include "FileManager.h"

FileManager::FileManager()
{
	_view = nullptr;
#ifdef __EMSCRIPTEN__
	_list.push_back("assets/examples/dark.pdb");
	_list.push_back("assets/examples/m0.1ps.pdb");
	_list.push_back("assets/examples/0.0ps.pdb");
	_list.push_back("assets/examples/0.1ps.pdb");
	_list.push_back("assets/examples/0.2ps.pdb");
	_list.push_back("assets/examples/0.3ps.pdb");
	_list.push_back("assets/examples/0.4ps.pdb");
	_list.push_back("assets/examples/0.5ps.pdb");
	_list.push_back("assets/examples/0.6ps.pdb");
	_list.push_back("assets/examples/3.0ps.pdb");
	_list.push_back("assets/examples/10.0ps.pdb");
	_list.push_back("assets/examples/50.0ps.pdb");
	_list.push_back("assets/examples/150.0ps.pdb");
#endif
}

void FileManager::setFilterType(File::Type type)
{
	_type = type;

	if (type == File::Nothing)
	{
		_filtered = _list;
		std::reverse(_filtered.begin(), _filtered.end());
		return;
	}
	
	_filtered.clear();

	for (size_t i = 0; i < _list.size(); i++)
	{
		if (valid(_list[i]))
		{
			_filtered.push_back(_list[i]);
		}
	}
	
	std::reverse(_filtered.begin(), _filtered.end());
}

bool FileManager::valid(std::string filename)
{
	File::Type type = File::typeUnknown(filename);

	return (type & _type);
}

void FileManager::addFile(std::string filename)
{
	_list.push_back(filename);

	if (_type == File::Nothing || valid(filename))
	{
		_filtered.push_back(filename);
	}
}

bool FileManager::acceptFile(std::string filename, bool force)
{
	bool added = false;

	if (file_exists(filename) || force)
	{
		addFile(filename);
		added = true;
	}
	
	if (_view != nullptr)
	{
		_view->filesChanged();
	}
	
	return added;
}

