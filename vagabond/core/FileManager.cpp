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
#include "Environment.h"
#include <fstream>
#include <sstream>
#include "commit.h"


std::string FileManager::_dataDir;
std::string FileManager::_userDir;

FileManager::FileManager()
{

}

void FileManager::correctFilename(std::string &filename)
{
	FileManager *fm = Environment::fileManager();
	std::string path = filename;
	
	if (file_exists(path))
	{
		return;
	}

	if (fm->_dataDir.length() > 0)
	{
		path = fm->_dataDir + "/" + filename;
	}

	if (!file_exists(path) && fm->_userDir.length() > 0)
	{
		path = fm->_userDir + "/" + filename;
	}

	if (!file_exists(path))
	{
		path = std::string(DATA_DIRECTORY) + "/" + filename;
	}

	filename = path;
}

std::set<std::string> &FileManager::geometryFiles()
{
	if (_geometries.size())
	{
		return _geometries;
	}
	
	setFilterType(File::Geometry);

	for (size_t i = 0; i < filteredCount(); i++)
	{
		std::string file = filtered(i);
		if (!file_exists(file))
		{
			continue;
		}

		_geometries.insert(file);
	}

	return _geometries;
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

bool FileManager::hasFile(std::string filename)
{
	std::vector<std::string>::iterator it;
	it = std::find(_list.begin(), _list.end(), filename);
	
	return (it != _list.end());
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
	bool already = std::find(_list.begin(), _list.end(), filename) != _list.end();

	if ((file_exists(filename) && !already) || force)
	{
		addFile(filename);
		added = true;
	}
	
	_geometries.clear();
	
	HasResponder<Responder<FileManager>>::triggerResponse();
	
	return added;
}

void FileManager::prepareDownload(void *me, void *data, int nbytes)
{
	char *tmp = new char[nbytes + 1];
	memcpy(tmp, data, nbytes * sizeof(char));
	tmp[nbytes] = '\0';

	acceptDownload(me, std::string(tmp));
}

void FileManager::acceptDownload(void *me, std::string contents)
{
	FileManager *fm = Environment::fileManager();
	fm->clickTicker();

	std::string &link = *static_cast<std::string *>(me);
	
	if (contents.rfind("HEADER", 0) == std::string::npos)
	{
		std::cout << "Skipping " << link << std::endl;
		return;
	}

	size_t slash = link.find("/", 10);
	slash++;
	slash = link.find("/", slash);
	slash++;
	std::string code = link.substr(slash, 4);
	to_lower(code);
	
	std::string filename = code + ".pdb";
	if (file_exists(filename))
	{
		std::cout << "filename " << filename << " already exists" << std::endl;
	}

	std::ofstream file;
	file.open(filename);
	file << contents;
	file.close();
	
	std::cout << filename << std::endl;
	fm->acceptFile(filename);
}

pthread_t &FileManager::thread()
{
	FileManager *fm = Environment::fileManager();
	return fm->_thread;
}
