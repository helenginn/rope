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

class Rule;
class Entity;
class Metadata;
class Molecule;
class FileManager;
class ModelManager;
class EntityManager;
class ProgressResponder;

class Environment
{
public:
	Environment();
	virtual ~Environment();

	static Metadata *metadata()
	{
		return _environment._metadata;
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
	
	ProgressResponder *progressResponder() const
	{
		return _pg;
	}
	
	void setProgressResponder(ProgressResponder *pg)
	{
		_pg = pg;
	}
	
	static void autoModel();
	
	void purgeMolecule(Molecule *mol);
	static void purgeModel(std::string name);
	static void purgeEntity(std::string name);
	static void purgeRule(Rule &rule);
	
	void save();
	void load(std::string file = "rope.json");
private:
	FileManager *_fileManager = nullptr;
	ModelManager *_modelManager = nullptr;
	EntityManager *_entityManager = nullptr;
	Metadata *_metadata = nullptr;
	
	ProgressResponder *_pg = nullptr;

	static Environment _environment;
};

#endif
