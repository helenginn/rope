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
#include <mutex>
#include <iostream>

#include "HasMetadata.h"
#include "Metadata.h"

#include "Polymer.h"
#include "Ligand.h"
#include "Responder.h"
#include "AtomGroup.h"
#include "CustomProtonSettings.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

class File;
class Chain;
class Polymer;
class Diffraction;
class AtomContent;
class ArbitraryMap;

// deliberately forward-declared, not #included: Clique.h pulls in Probe.h
// and its heavy Connector/hnet chain, and Model.h is included almost
// everywhere - see Model.cpp for the handful of methods that actually
// need Clique's complete type.
class Clique;

class Model : public HasResponder<Responder<Model>>, Responder<AtomGroup>,
public HasMetadata
{
public:
	Model();

	// declared (not defaulted) here, defined in Model.cpp - _cliques is a
	// std::list<Clique> held by value, and Clique is only forward-declared
	// above, so the compiler-generated bodies for these (which need
	// Clique's complete type to destroy/copy/move that member) must be
	// instantiated in Model.cpp, where Clique.h is actually included.
	~Model();
	Model(const Model &other);
	Model(Model &&other) noexcept;
	Model &operator=(const Model &other);
	Model &operator=(Model &&other) noexcept;

	static Model autoModel(std::string filename);
	
	void setFilename(std::string file);
	
	const std::string entityForChain(std::string id) const;
	void setEntityForChain(std::string id, std::string entity);
	
	bool hasEntity(std::string entity) const;
	size_t instanceCountForEntity(std::string entity) const;

	void throwOutInstance(Instance *inst);
	void throwOutEntity(Entity *ent);

	void autoAssignEntities(Entity *chosen = nullptr);
	
	virtual bool displayable() const
	{
		return false;
	}
	
	std::set<std::string> ents() const;
	
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

	std::set<Entity *> entities();
	std::set<Polymer *> polymersForEntity(Entity *ent);

	Polymer *polymerFromChain(Chain *ch);
	
	std::list<Polymer> &polymers()
	{
		return _polymers;
	}
	
	std::vector<Instance *> instances(); // do not attempt to make const!
	Instance *instanceWithId(std::string id);

	void housekeeping();
	
	void createPolymers();
	
	enum LoadOptions
	{
		Everything = 0,
		NoGeometry = 1 << 0,
		NoAngles = 1 << 1,
		NoInsert = 1 << 2,
	};

	void load(LoadOptions opts = Everything);
	bool unload();
	void reload();
	
	int loadCount() const
	{
		return _loadCounter;
	}
	
	bool loaded() const
	{
		return (_currentAtoms != nullptr);

	}

	void refine(bool sameThread = false, bool thorough = false);
	
	AtomContent *const currentAtoms() const
	{
		return _currentAtoms;
	}

	friend void to_json(json &j, const Model &value);
	friend void from_json(const json &j, Model &value);
	
	virtual const Metadata::KeyValues metadata(Metadata *source = nullptr) const;
	
	void clickTicker();
	void findInteractions();
	void finishedRefinement();
	void write(std::string filename);

	void extractExisting();
	Diffraction *getDiffraction(std::string filename = "");

	Polymer *polymerForChain(std::string ch) const
	{
		return _chain2Polymer.at(ch);
	}

	virtual void respond();

	void export_refined(std::string prefix = "rope", std::string suffix = "");

	const std::list<Clique> &cliques() const;
	void setCliques(const std::list<Clique> &cliques);

	CustomProtonSettings &protonSettings()
	{
		return _protonSettings;
	}

	const CustomProtonSettings &protonSettings() const
	{
		return _protonSettings;
	}
private:
	void swapChainToEntity(std::string id, std::string entity);
	void mergeAppropriatePolymers();
	bool mergePolymersInSet(std::set<Polymer *> polymers);
	void assignSequencedPolymers(Entity *chosen);
	/* assign waters, ligands etc. */
	void assignClutter();

	void extractTorsions();
	void insertTorsions();
	std::string _filename;
	std::string _dataFile;
	std::string _name;

	std::map<std::string, std::string> _chain2Entity;
	std::map<std::string, Polymer *> _chain2Polymer;

	std::list<Polymer> _polymers;
	std::list<Ligand> _ligands;
	std::list<Clique> _cliques;

	CustomProtonSettings _protonSettings;

	int _loadCounter = 0;
	std::mutex *_loadMutex = nullptr;
	File *_currentFile = nullptr;
	AtomContent *_currentAtoms = nullptr;
	bool _everything =false;
	LoadOptions _loadOptions {};
};

// defined in Model.cpp, not here - both touch value._cliques, which needs
// Clique's complete type (see the forward declaration above).
void to_json(json &j, const Model &value);
void from_json(const json &j, Model &value);

#endif
