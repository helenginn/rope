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

#include "ModelManager.h"
#include "FileManager.h"
#include "Environment.h"
#include <iostream>

ModelManager::ModelManager() : Manager()
{
	_mutex = new std::mutex();

}

Model *ModelManager::insertIfUnique(Model &m)
{
    std::string model_name = m.name();
    to_lower(model_name);
    if (_name2Model.count(model_name) > 0)
    {
        throw std::runtime_error("Model with same name exists");
    }
	
	if (m.name().length() == 0)
	{
		throw std::runtime_error("Model has no name");
	}
	
	if (m.filename().length() == 0)
	{
		throw std::runtime_error("Model has no initial file");
	}
	
	_objects.push_back(m);
	housekeeping();

	Manager::triggerResponse();
	
	return &_objects.back();
}

void ModelManager::update(const Model &m)
{
	Model *old = model(m.name());
	*old = m;
}

void ModelManager::housekeeping()
{
	for (Model &m : _objects)
	{
        std::string model_name = m.name();
        to_lower(model_name);
		_name2Model[model_name] = &m;
	}
	
	connectionsToDatabase();
}

void ModelManager::rescan()
{
	_mutex->lock();

	for (Model &m : objects())
	{
		clickTicker();
		m.autoAssignEntities();
	}

	finishTicker();
	_mutex->unlock();
}

void ModelManager::autoModel()
{
	_mutex->lock();
	FileManager *fm = Environment::fileManager();
	fm->setFilterType(File::MacroAtoms);
	
	std::vector<std::string> list = fm->filteredList();
	
	for (size_t i = 0; i < list.size(); i++)
	{
		clickTicker();
		std::string filename = list[i];
		Model model = Model::autoModel(filename);
		try
		{
			Model *ptr = insertIfUnique(model);
			ptr->autoAssignEntities();
			ptr->findInteractions();
		}
		catch (const std::runtime_error &err)
		{
			std::cout << err.what() << " - skipping." << std::endl;
		}
	}

	fm->setFilterType(File::Nothing);
	housekeeping();
	finishTicker();
	_mutex->unlock();
}

Model *ModelManager::modelUsingFilename(std::string &filename)
{
	for (Model &m : _objects)
	{
		if (m.filename() == filename)
		{
			return &m;
		}
	}

	return nullptr;
}

void ModelManager::purgeModel(Model *model)
{
	std::list<Model>::iterator it = _objects.begin();

	std::string name = model->name();
	_name2Model.erase(name);

	for (Model &m : _objects)
	{
		if (model == &m)
		{
			_objects.erase(it);
			return;
		}

		it++;
	}
	
	Manager::triggerResponse();
}

void ModelManager::purgeInstance(Instance *inst)
{
	for (Model &m : _objects)
	{
		m.throwOutInstance(inst);
	}
}

void ModelManager::purgeEntity(Entity *ent)
{
	for (Model &m : _objects)
	{
		m.throwOutEntity(ent);
	}
}

void ModelManager::connectionsToDatabase()
{
	for (Model &m : _objects)
	{
		std::vector<Instance *> instances = m.instances();
		for (Instance *inst : instances)
		{
			inst->housekeeping();
			std::string mol_id = inst->id();
			std::string mod_id = inst->model_id();
			Environment::metadata()->setModelIdForInstanceId(mol_id, mod_id);
		}
	}
}
