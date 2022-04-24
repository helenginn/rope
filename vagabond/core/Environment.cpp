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

#include "Environment.h"
#include "FileManager.h"
#include "ModelManager.h"
#include "EntityManager.h"

#include <iostream>
#include <fstream>
#include <json/json.hpp>
using nlohmann::json;

Environment Environment::_environment;

Environment::Environment()
{
	_fileManager = new FileManager();
	_modelManager = new ModelManager();
	_entityManager = new EntityManager();
}

void Environment::save()
{
	json data;
	data["file_manager"] = *_fileManager;
	data["model_manager"] = *_modelManager;
	data["entity_manager"] = *_entityManager;
	
	std::ofstream file;
	file.open("rope.json");
	file << data;
	file << std::endl;
	file.close();
}

void Environment::load(std::string file)
{
	json data;
	
	std::ifstream f;
	f.open(file);
	f >> data;
	f.close();

	try
	{
		*_fileManager = data["file_manager"];
		 *_modelManager = data["model_manager"];
		 *_entityManager = data["entity_manager"];
	}
	catch (const nlohmann::detail::type_error &err)
	{
		std::cout << "Error processing json, probably old version" << std::endl;
	}
	
	_modelManager->housekeeping();
}
