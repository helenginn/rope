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

#ifndef __vagabond__Model__
#define __vagabond__Model__

#include <string>
#include <list>
#include <iostream>

#include "HasMetadata.h"
#include "Metadata.h"

#include "Molecule.h"
#include "AtomGroup.h"
#include "AtomRecall.h"

#include <json/json.hpp>
using nlohmann::json;

class File;
class Chain;
class Molecule;
class AtomContent;

class ModelResponder
{
public:
	virtual ~ModelResponder() {}
	virtual void modelReady() = 0;
};

class Model : public AtomGroupResponder, public HasMetadata
{
public:
	Model();
	
	static Model autoModel(std::string filename);
	
	void setResponder(ModelResponder *responder)
	{
		_responder = responder;
	}
	
	void setFilename(std::string file);
	
	const std::string entityForChain(std::string id) const;
	void setEntityForChain(std::string id, std::string entity);
	
	bool hasEntity(std::string entity) const;
	size_t moleculeCountForEntity(std::string entity) const;

	void throwOutMolecule(Molecule *mol);
	void throwOutEntity(Entity *ent);

	void autoAssignEntities(Entity *chosen = nullptr);
	
	void setName(std::string name)
	{
		_name = name;
	}

	const std::string &filename() const
	{
		return _filename;
	}
	
	void setDataFile(const std::string name)
	{
		_dataFile = name;
	}

	const std::string &dataFile() const
	{
		return _dataFile;
	}

	const std::string &name() const
	{
		return _name;
	}

	virtual const std::string id() const
	{
		return name();
	}

	Molecule *moleculeFromChain(Chain *ch);
	
	std::list<Molecule> &molecules()
	{
		return _molecules;
	}

	void housekeeping();
	
	void createMolecules();

	void load();
	void unload();

	void refine();
	
	AtomContent *currentAtoms()
	{
		return _currentAtoms;
	}

	void distanceBetweenAtoms(Entity *ent, AtomRecall &a, AtomRecall &b,
	                          std::string header, Metadata *md);
	void angleBetweenAtoms(Entity *ent, AtomRecall &a, AtomRecall &b,
	                       AtomRecall &c, std::string header, Metadata *md);

	friend void to_json(json &j, const Model &value);
	friend void from_json(const json &j, Model &value);
	
	virtual const Metadata::KeyValues metadata() const;
	
	void clickTicker();
	void finishedRefinement();
private:
	void swapChainToEntity(std::string id, std::string entity);
	void extractTorsions();
	void insertTorsions();
	std::string _filename;
	std::string _dataFile;
	std::string _name;

	std::map<std::string, std::string> _chain2Entity;
	std::map<std::string, Molecule *> _chain2Molecule;

	std::list<Molecule> _molecules;

	File *_currentFile = nullptr;
	AtomContent *_currentAtoms = nullptr;
	
	ModelResponder *_responder = nullptr;
};

inline void to_json(json &j, const Model &value)
{
	j["name"] = value._name;
	j["filename"] = value._filename;
	j["chain_to_entity"] = value._chain2Entity;
	j["molecules"] = value._molecules;
	j["datafile"] = value._dataFile;
}

inline void from_json(const json &j, Model &value)
{
	value._name = j.at("name");
	value._filename = j.at("filename");
	
	try
	{
		value._chain2Entity = j.at("chain_to_entity");
		value._molecules = j.at("molecules");
		value._dataFile = j.at("dataFile");
	}
	catch (...)
	{
		std::cout << "Error proccessing json, probably old version" << std::endl;
	}
	
	value.clickTicker();
}



#endif
