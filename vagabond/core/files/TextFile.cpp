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

#include "files/TextFile.h"
#include "Metadata.h"
#include <fstream>
#include <vagabond/utils/FileReader.h>
#include <iostream>

TextFile::TextFile(std::string filename) : File(filename)
{

}

File::Type TextFile::cursoryLook()
{
	std::string path = toFilename(_filename);
	
	if (!file_exists(path))
	{
		std::cout << "Could not open file: " << _filename << std::endl;
		return Nothing;
	}

	std::ifstream f;
	f.open(path);
	
	if (!f.is_open())
	{
		std::cout << "Could not open file: " << _filename << std::endl;
		return Nothing;
	}

	std::string line;
	getline(f, line);
	defenestrate(line);
	debom(line);
	
	if (line.length() == 0)
	{			
		f.close();
		return Nothing;
	}
	
	std::vector<std::string> headers = split(line, ' ');
	

	for (std::string &header : headers)
	{
		trim(header);
		remove_quotes(header);
		trim(header);
		if (header == "H-bond_ID")
		{
			f.close();
			return Meta;
		}
	}

	f.close();
	return Nothing;
}

void TextFile::processLine(std::string line)
{
	std::vector<std::string> components = split(line, ' ');
	
	Metadata::KeyValues kvs;
	for (size_t i = 0; i < components.size() && i < _headers.size(); i++)
	{
		std::string h = _headers[i];
		
		if (components[i].length())
		{
			defenestrate(components[i]);
			trim(components[i]);
			remove_quotes(components[i]);
		}

		if (components[i].length() == 0)
		{
			continue;
		}


		kvs[h] = Value(components[i]);
	}
	_metadata->addKeyValues(kvs, true);
}

void TextFile::processHeaders(std::string line)
{
	_headers = split(line, ' ');

	for (std::string &h : _headers)
	{
		defenestrate(h);
		remove_quotes(h);
	}
	std::cout << std::endl;
	
}

void TextFile::parseFileContents()
{
	_metadata->setSource(_filename);

	std::string path = toFilename(_filename);

	std::ifstream f;
	f.open(path);
	
	if (!f.is_open())
	{
		std::cout << "Could not open file: " << _filename << std::endl;
		return;
	}

	std::string line;
	if (!getline(f, line))
	{
		return;
	}
	
	debom(line);
	processHeaders(line);

	while (getline(f, line))
	{
		processLine(line);
	}
}

void TextFile::parse()
{
	parseFileContents();
}
