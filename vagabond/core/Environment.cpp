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
#include "HBondData.h"
#include "Ruler.h"
#include "PathManager.h"
#include "ModelManager.h"
#include "EntityManager.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using nlohmann::json;

Environment Environment::_environment;

Environment::Environment()
{
	_metadata = new Metadata();
	_metadata->setSource("master");
	_hBondData = new HBondData();
	_hBondData->setSource("master");
	_fileManager = new FileManager();
	_pathManager = new PathManager();
	_modelManager = new ModelManager();
	_entityManager = new EntityManager();
}

Environment::~Environment()
{

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
	data["hBondData"] = *_hBondData;
#ifdef __EMSCRIPTEN__
	std::string contents = data.dump();
#endif
	
	std::ofstream file;
	file.open("rope.json");
	file << data.dump(1);
	file << std::endl;
	file.close();

#ifdef __EMSCRIPTEN__
	EM_ASM_({ window.download = download; window.download($0, $1, $2) }, 
	        "rope.json", contents.c_str(), contents.length());
#endif
}

void Environment::loadEntitiesBackwardsCompatible(const json &data)
{
	try
	{
		*_entityManager = data["entity_manager"];
	}
	catch (const json::exception &err)
	{
		// old version of RoPE, before split between polymers and entities.
		std::cout << "Trying to fix problem" << std::endl;
		_entityManager = new EntityManager();
		_entityManager->setPolymerEntityManager(data["entity_manager"]);
	}
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
	
	*_fileManager = data["file_manager"];
	*_modelManager = data["model_manager"];
	loadEntitiesBackwardsCompatible(data);
	*_metadata = data["metadata"];

	  // Check if hBondData key exists
	if (data.count("hBondData")) 
	{
		*_hBondData = data["hBondData"];
	} 
	else 
	{
		// hBondData key missing, initialize an empty HBondData object
		_hBondData = new HBondData();
		_hBondData->setSource("master"); // Set source if necessary
		std::cerr << "Warning: hBondData key missing in JSON file. Using an empty HBondData object." << std::endl;
	}

	*_pathManager = data["path_manager"];

	_metadata->housekeeping();
	_hBondData->housekeeping();
	_entityManager->housekeeping();
	_modelManager->housekeeping();
	
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
	EntityManager *em = Environment::entityManager();
	
	mm->rescan();
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

void Environment::purgeInstance(Instance *inst)
{
	entityManager()->purgeInstance(inst);
	modelManager()->purgeInstance(inst);
}

void Environment::purgeEntity(std::string id)
{
	Entity *ent = entityManager()->entity(id);
	if (ent)
	{
		pathManager()->purgeEntity(ent);
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
		std::vector<Instance *> insts = model->instances();
		for (Instance *inst : insts)
		{
			inst->flagPurge();
		}

		entityManager()->purgeModel(model);
		modelManager()->purgeModel(model);
	}
}

void Environment::purgePath(Path &path)
{
	pathManager()->purgePath(&path);
}

