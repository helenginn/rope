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
#include "ModelManager.h"
#include <fstream>
#include <sstream>
#include "config/config.h"


std::string FileManager::_dataDir;
std::string FileManager::_userDir;

FileManager::FileManager()
{

}

void FileManager::correctFilename(std::string &filename)
{
    // Get the file manager instance
    FileManager *fm = Environment::fileManager();

    // Check if the file exists in the current directory
    std::filesystem::path candidate(filename);
    if (std::filesystem::exists(candidate))
    {
        // Normalize the path and update filename
        filename = candidate.lexically_normal().string();
        return;
    }

    // Construct a set of potential directories to check
    std::vector<std::filesystem::path> lookupDirs;
    if (!fm->_dataDir.empty()) lookupDirs.push_back(std::filesystem::path(fm->_dataDir));
    if (!fm->_userDir.empty()) lookupDirs.push_back(std::filesystem::path(fm->_userDir));
    if (!std::string(DATA_DIRECTORY).empty()) lookupDirs.push_back(std::filesystem::path(DATA_DIRECTORY));

    // Iterate through the potential directories and return the first one where the file exists
    for (const auto& dir : lookupDirs)
    {
        std::filesystem::path p = dir / filename;
        if (std::filesystem::exists(p))
        {
            filename = p.lexically_normal().string();
            return;
        }
    }

    // If the file was not found in any of the directories, leave filename unchanged
    filename = candidate.lexically_normal().string();
}

std::set<std::string> &FileManager::geometryFiles()
{
	if (_foundGeometries)
	{
		return _geometries;
	}
	
	setFilterType(File::Geometry);

	for (size_t i = 0; i < filteredCount(); i++)
	{
		std::string file = filtered(i);
		if (!FileReader::file_exists(file))
		{
			continue;
		}

		_geometries.insert(file);
	}

	setFilterType(File::Nothing);
	_foundGeometries = true;

	return _geometries;
}

void FileManager::preFilter()
{
	for (size_t i = 0; i < _list.size(); i++)
	{
		valid(_list[i]);
	}
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
	if (_filename2Type.count(filename))
	{
		return (_type & _filename2Type[filename]);
	}

	File::Type type = File::typeUnknown(filename);
	_filename2Type[filename] = type;

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
    std::cout << "Adding file " << filename << std::endl;
	_list.push_back(filename);

	if (_type == File::Nothing || valid(filename))
	{
	    std::cout << "Adding file to filtered list " << filename << std::endl;
		_filtered.push_back(filename);
	}
}

bool FileManager::acceptFile(std::string filename, bool force)
{
	bool added = false;
	bool already = std::find(_list.begin(), _list.end(), filename) != _list.end();

	if ((FileReader::file_exists(filename) && !already) || force)
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
//	fm->clickTicker();

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
	if (FileReader::file_exists(filename))
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

void FileManager::loadGlobFiles()
{
	std::vector<std::string> globs = FileReader::glob_pattern("*");

	for (std::string &g : globs)
	{
		loadFile(g);
	}
}

void FileManager::loadFile(std::string &file)
{
	escape_filename(file);
	acceptFile(file);
}

int FileManager::unloadMissingFiles(bool wipeUsing)
{
	int count = 0;
	int deleted = 0;
	ModelManager *mm = Environment::modelManager();

	for (size_t i = 0; i < _list.size(); i++)
	{
		std::string &file = _list[i];
		if (FileReader::file_exists(file))
		{
			continue;
		}
		
		Model *aModel = mm->modelUsingFilename(file);
		if (!wipeUsing && aModel)
		{
			count++;
			continue;
		}
		
		/* throw away all models which use this filename */
		while (aModel)
		{
			mm->purgeModel(aModel);
			aModel = mm->modelUsingFilename(file);
		}
		
		_list.erase(_list.begin() + i);
		i--;
		deleted++;
	}

	std::cout << "Deleted " << deleted << " files" << std::endl;

	setFilterType(File::Nothing);
	HasResponder<Responder<FileManager>>::triggerResponse();
	return count;
}
