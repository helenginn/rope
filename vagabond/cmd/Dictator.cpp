// breathalyser
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

#include "../utils/FileReader.h"
#include "Dictator.h"
#include "CmdWorker.h"
#include <vagabond/core/FileManager.h>
#include <vagabond/core/Environment.h>
#include <iostream>

std::map<std::string, std::string> Dictator::_properties;
std::map<std::string, std::string> Dictator::_commands;

void Dictator::makeCommands()
{
	_commands["load"] = "Comma- or space-separated list of files to load";
	_commands["environment"] = ("Link to json file (usually rope.json) to"\
	                            "restore RoPE environment");
}

void splitCommand(std::string command, std::string *first, std::string *last)
{
	size_t equal = command.find('=');
	if (equal == std::string::npos)
	{
		*first = command;
		*last = "";
		return;
	}

	*first = command.substr(0, equal);
	
	if (command.length() <= equal + 1)
	{
		*last = "";
		return;
	}

	*last = command.substr(equal + 1, std::string::npos);
}

Dictator::Dictator()
{
	_worker = NULL;
	_currentJob = -1;
	makeCommands();
}

void Dictator::setup()
{
	if (_worker != nullptr)
	{
		delete _worker;
	}

	_worker = new CmdWorker(this);
}

void Dictator::loadFiles(std::string &last)
{
	FileManager *fm = Environment::fileManager();
	std::vector<std::string> files = split(last, ',');

	for (size_t i = 0; i < files.size(); i++)
	{
		if (!fm->acceptFile(files[i]))
		{
			std::cout << "File not found/accessible: " << 
			files[i] << std::endl;
		}
	}
}

void Dictator::processRequest(std::string &first, std::string &last)
{
	if (first == "load" || first == "")
	{
		loadFiles(last);
	}

	if (first == "environment" || first == "")
	{
		Environment::env().load(last);
	}
}

bool Dictator::checkForFile(std::string &first, std::string &last)
{
	if (last != "")
	{
		return false;
	}
	
	if (file_exists(first))
	{
		loadFiles(first);
		return true;
	}
	
	return false;
}

bool Dictator::sanityCheck(std::string &first, std::string &last)
{
	if (_commands.count(first) == 0)
	{
		std::cout << "Command not recognised: " << first << std::endl;
		return false;
	}

	return true;
}

void Dictator::processNextArg(std::string arg)
{
	std::cout << std::endl;
	std::cout << "Processing next arg: " << "\"" << arg << 
	"\"" << std::endl;
	std::string first, last;
	splitCommand(arg, &first, &last);
	
	if (checkForFile(first, last))
	{
		return;
	}
	
	if (sanityCheck(first, last))
	{
		processRequest(first, last);
	}
}

bool Dictator::nextJob()
{
	if (_args.size() == 0)
	{
		Environment::env().load();
		return false;
	}
	
	_currentJob++;

	if ((size_t)_currentJob >= _args.size())
	{
		return false;
	}

	processNextArg(_args[_currentJob]);
	return true;
}

void Dictator::start()
{
	if (_worker == nullptr)
	{
		setup();
	}

	_currentJob = -1;
	_thread = new std::thread(&CmdWorker::start, _worker);
}

