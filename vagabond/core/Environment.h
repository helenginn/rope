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

#ifndef __vagabond__Environment__
#define __vagabond__Environment__

#include <string>
#include <nlohmann/json.hpp>
using nlohmann::json;

class Path;
class Rule;
class Entity;
class Metadata;
class Instance;
class Progressor;
class FileManager;
class PathManager;
class ModelManager;
class EntityManager;

template <typename Progressor> class Responder;

class Environment
{
public:
	Environment();
	virtual ~Environment();

	static Metadata *metadata()
	{
		return _environment._metadata;
	}

	static PathManager *pathManager()
	{
		return _environment._pathManager;
	}

	static ModelManager *modelManager()
	{
		return _environment._modelManager;
	}

	static size_t entityCount();

	static EntityManager *entityManager()
	{
		return _environment._entityManager;
	}

	static FileManager *fileManager()
	{
		return _environment._fileManager;
	}

	static Environment &env()
	{
		return _environment;
	}

	static Environment &environment()
	{
		return _environment;
	}
	
	Responder<Progressor> *progressResponder() const
	{
		return _pg;
	}
	
	void setProgressResponder(Responder<Progressor> *pg)
	{
		_pg = pg;
	}
	
	static void autoModel();
	static void rescanModels();
	
	void purgeInstance(Instance *inst);
	static void purgeModel(std::string name);
	static void purgeEntity(std::string name);
	static void purgeRule(Rule &rule);
	static void purgePath(Path &path);
	
	void save();
	void load(std::string file = "rope.json");
private:
	void loadEntitiesBackwardsCompatible(const json &data);

	FileManager *_fileManager = nullptr;
	ModelManager *_modelManager = nullptr;
	EntityManager *_entityManager = nullptr;
	PathManager *_pathManager = nullptr;
	Metadata *_metadata = nullptr;
	
	Responder<Progressor> *_pg = nullptr;

	static Environment _environment;
};

#endif
