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
#include "Metadata.h"
#include "Ruler.h"
#include "PathManager.h"
#include "ModelManager.h"
#include "EntityManager.h"

#include <iostream>
#include <fstream>
#include <json/json.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using nlohmann::json;

Environment Environment::_environment;

Environment::Environment()
{
	_metadata = new Metadata();
	_metadata->setSource("master");

	_fileManager = new FileManager();
	_pathManager = new PathManager();
	_modelManager = new ModelManager();
	_entityManager = new EntityManager();
}

size_t Environment::entityCount()
{
	return Environment::entityManager()->objectCount();
}

void Environment::save()
{
	json data;
	data["file_manager"] = *_fileManager;
	data["model_manager"] = *_modelManager;
	data["entity_manager"] = *_entityManager;
	data["path_manager"] = *_pathManager;
	data["metadata"] = *_metadata;
	std::string contents = data.dump();
	
	std::ofstream file;
	file.open("rope.json");
	file << data;
	file << std::endl;
	file.close();

#ifdef __EMSCRIPTEN__
	EM_ASM_({ window.download = download; window.download($0, $1, $2) }, 
	        "rope.json", contents.c_str(), contents.length());
#endif
}

void Environment::load(std::string file)
{
	if (!file_exists(file))
	{
		std::cout << "Could not find json environment." << std::endl;
		if (_pg)
		{
			std::cout << "finish!" << std::endl;
			_pg->sendObject("finish", nullptr);
		}

		return;
	}

	std::cout << "Loading json environment " << file << "..." << std::endl;
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
		*_metadata = data["metadata"];
		*_pathManager = data["path_manager"];
	}
	catch (const nlohmann::detail::type_error &err)
	{
		std::cout << "Error processing json, probably old version" << std::endl;
	}
	
	_metadata->housekeeping();
	_entityManager->housekeeping();
	_modelManager->housekeeping();
	_pathManager->housekeeping();
	
	_entityManager->checkModelsForReferences(_modelManager);
	std::cout << "done." << std::endl;
	
	if (_pg)
	{
		_pg->sendObject("finish", nullptr);
	}
}

void Environment::rescanModels()
{
	ModelManager *mm = Environment::modelManager();
	
	for (Model &m : mm->objects())
	{
		mm->clickTicker();
		m.autoAssignEntities();
	}

	EntityManager *em = Environment::entityManager();
	em->checkModelsForReferences(mm);
	mm->finishTicker();
}

void Environment::autoModel()
{
	ModelManager *mm = Environment::modelManager();
	EntityManager *em = Environment::entityManager();
	mm->autoModel();
	em->checkModelsForReferences(mm);
	
	mm->finishTicker();
}

void Environment::purgeMolecule(Molecule *mol)
{
	entityManager()->purgeMolecule(mol);
	modelManager()->purgeMolecule(mol);
}

void Environment::purgeEntity(std::string id)
{
	Entity *ent = entityManager()->entity(id);
	if (ent)
	{
		modelManager()->purgeEntity(ent);
		entityManager()->purgeEntity(ent);
	}
}

void Environment::purgeRule(Rule &rule)
{
	Metadata *md = metadata();
	md->ruler().removeLike(rule);
}

void Environment::purgeModel(std::string id)
{
	Model *model = modelManager()->model(id);
	if (model)
	{
		entityManager()->purgeModel(model);
		modelManager()->purgeModel(model);
	}
}

void Environment::purgePath(Path &path)
{
	pathManager()->remove(path);
}

Environment::~Environment()
{

}
