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

#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/os.h>
#include "Dictator.h"
#include "Socket.h"
#include "CmdWorker.h"
#include <vagabond/core/Environment.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/ModelManager.h>
#include <vagabond/core/Reporter.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>

std::map<std::string, std::string> Dictator::_properties;
std::map<std::string, std::string> Dictator::_commands;

void Dictator::makeCommands()
{
	_commands["load"] = "Comma- or space-separated list of files to load";
	_commands["exit"] = "Exit RoPE";
	_commands["add"] = "Load metadata CSV file with 'model' or 'filename' column";
	_commands["hungry-hippo"] = "Begin hungry-hippo mode (e.g. for beamtime)";

	_commands["path-matrix"] = "Matrix of thermodynamic scores for paths between list of instances. First argument: output filename; all following arguments: instance identifiers";
	_commands["refine-path"] = "Refine between instances (first and second argument), for N cycles (third argument, default 1)";
	_commands["auto-paths"] = "Refine all pairs within a group of instances. First argument: integer number of cycles; all following arguments: instance identifiers";
	_commands["save"] = "Save to environment file.";

	_commands["environment"] = ("Link to json file (usually rope.json) to "\
	                            "restore RoPE environment");
	_commands["automodel"] = ("Auto-model atom coordinate files according "\
	                          "to existing entities matching at least 80% "
	                          "sequence and default names");
	_commands["rescan"] = ("Rescan existing models for existing entities"
	                          " matching at least 80% sequence and default names");
	_commands["get-files-native-app"] = ("Load files from three directories up in "
	                                     "the Mac Rope app file");
    _commands["report"] = ("Report various statistics on the existing environment. Useful for debugging.");
    _commands["export"] = ("Export a model as a .pdb file");
    _commands["--help"] = ("Displays available commands.");
    _commands["-h"] = ("Displays available commands.");
    _commands["help"] = ("Displays available commands.");
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

void Dictator::getFilesNativeApp()
{
	FileManager *fm = Environment::fileManager();
	fm->loadGlobFiles();
}

void Dictator::loadFiles(std::string &last)
{
	FileManager *fm = Environment::fileManager();
	std::vector<std::string> files = split(last, ',');

	for (size_t i = 0; i < files.size(); i++)
	{
		std::string file = files[i];
		escape_filename(file);

		if (!fm->acceptFile(file))
		{
			std::cout << "File not found/accessible: " << 
			file << std::endl;
		}
		else
		{
			std::cout << "Found file: " << file << std::endl;
		}
	}
}

void Dictator::processRequest(std::string &first, std::string &last)
{
	if (first == "--help" || first == "-h" || first == "help")
    {
        std::map<std::string, std::string>::iterator i;
        for (i = _commands.begin(); i != _commands.end(); i++)
        {
            if (i->first == "-h" || i->first == "help")
            {
                continue;
            }
            else if (i->first == "--help")
            {
                std::cout << i->first << ", -h, help" << std::endl << "     " << i->second << std::endl;
            }
            else
            {
                std::cout << i->first << std::endl << "     " << i->second << std::endl;
            }
        }
        exit(0);
    }

    if (first == "load" || first == "")
	{
		loadFiles(last);
	}
	
	if (first == "exit")
	{
		exit(0);
	}
	
	if (first == "save")
	{
		Environment::env().save();
		std::cout << "Saved." << std::endl;
	}

	if (first == "environment")
	{
		Environment::env().load(last);
	}

	if (first == "path-matrix")
	{
		std::vector<std::string> args = split(last, ',');
		std::string filename = args[0];
		args.erase(args.begin());
		PathManager::manager()->pathMatrix(filename, args);
	}

	if (first == "auto-paths")
	{
		std::vector<std::string> args = split(last, ',');
		int num = atoi(args[0].c_str());
		args.erase(args.begin());

		PathManager::manager()->makePathsWithinGroup(args, num);
	}

	if (first == "refine-path")
	{
		std::cout << "here" << std::endl;
		std::vector<std::string> args = split(last, ',');
		if (args.size() < 2)
		{
			std::cout << "Not enough arguments to refine path" << std::endl;
		}
		std::string &start = args[0];
		std::string &end = args[1];
		
		int cycles = 1;
		if (args.size() > 2)
		{
			cycles = atoi(args[2].c_str());
		}
		PathManager::manager()->makePathBetween(start, end, cycles);
	}

	if (first == "automodel")
	{
		Environment::env().autoModel();
	}

	if (first == "get-files-native-app")
	{
		getFilesNativeApp();
	}

	if (first == "rescan")
	{
		Environment::rescanModels();
	}

	if (first == "add")
	{
		File *file = File::loadUnknown(last);
		
		File::Type type = File::Nothing;
		
		if (file)
		{
			type = file->cursoryLook();
		}

		if (type & File::Meta)
		{
			*Environment::metadata() += *file->metadata();
		}
		else
		{
			std::cout << "File " << last << " is not readable or is "
			"not metadata." << std::endl;
		}
	}

    if (first == "export")
    {
        ModelManager *model_manager = Environment::env().modelManager();
        if (last == "all" || last == "")
        {
            std::cout << "Exporting all models..." << std::endl;
            for (int i = 0; i < model_manager->objectCount(); i++)
            {
                Model &model = model_manager->object(i);
                std::cout << "Exporting model " << model.id() << "... " << std::endl;
                model.export_refined();
                std::cout << "Done." << std::endl;
            }
        }
        else
        {
            std::cout << "Exporting model " << last << std::endl;
            Model *model = model_manager->model(last);
            if (model == nullptr)
            {
                to_lower(last);
                std::cout << "No model named: " << last << std::endl;
            }
            else
            {
                model->export_refined();
            }
        }
    }

    if (first == "report")
    {
        Reporter reporter;
        reporter.report();
    }

	if (first == "hungry-hippo")
	{
		delete _socket;
		_socket = new Socket(this);
		_socketer = new std::thread(&Socket::operator(), _socket);
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
//	std::cout << std::endl;
//	std::cout << "Processing next arg: " << "\"" << arg << 
//	"\"" << std::endl;
	std::string first, last;
	splitCommand(arg, &first, &last);
//	std::cout << first << " " << last << std::endl;
	
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
	_currentJob++;

	if ((size_t)_currentJob >= _args.size())
	{
		_currentJob--;
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

void Dictator::wait()
{
    _thread->join();
}

void sanitise(std::string &argument)
{
	if (argument == "hungry-hippo")
	{
		argument = "";
	}
}

void Dictator::sendObject(std::string tag, void *object)
{
	if (tag == "arg")
	{
		std::string argument = *static_cast<std::string *>(object);
		sanitise(argument);
		
		if (argument.length())
		{
			addArg(argument);
		}
	}

}
