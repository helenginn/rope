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

#include "Model.h"
#include "Molecule.h"
#include "Chain.h"
#include "File.h"
#include "AtomContent.h"
#include "../utils/FileReader.h"

Model::Model()
{

}

void Model::setFilename(std::string file)
{
	_filename = file;
	
	if (_name.length() == 0)
	{
		_name = getBaseFilename(_filename);
	}

}

const std::string Model::entityForChain(std::string id) const
{
	if (_chain2Entity.count(id))
	{
		return _chain2Entity.at(id);
	}

	return "";
}

void Model::setEntityForChain(std::string id, std::string entity)
{
	_chain2Entity[id] = entity;
}

bool Model::hasEntity(std::string entity)
{
	std::map<std::string, std::string>::iterator it;
	
	for (it = _chain2Entity.begin(); it != _chain2Entity.end(); it++)
	{
		if (it->second == entity)
		{
			return true;
		}
	}
	
	return false;
}

Molecule *Model::moleculeFromChain(Chain *ch)
{
	std::string entity = entityForChain(ch->id());
	if (entity == "")
	{
		return nullptr;
	}

	Molecule mc(name(), entity, ch->fullSequence());
	_molecules.push_back(mc);
	
	Molecule &ref = _molecules.back();

	ref.setChain(ch->id());
	ref.getTorsionRefs(ch);
	
	return &_molecules.back();
}

void Model::removeReferences()
{
	if (_currentFile)
	{
		delete _currentAtoms;
		delete _currentFile;
	}

	_currentFile = nullptr;
	_currentAtoms = nullptr;
}

void Model::createMolecules()
{
	std::map<std::string, std::string>::iterator it;
	
	for (it = _chain2Entity.begin(); it != _chain2Entity.end(); it++)
	{
		std::string id = it->first;
		if (_chain2Molecule.count(id) > 0)
		{
			continue;
		}
		
		if (_currentFile == nullptr)
		{
			_currentFile = File::loadUnknown(_filename);
			_currentAtoms = _currentFile->atoms();
		}

		Chain *ch = _currentAtoms->chain(id);
		
		if (ch)
		{
			moleculeFromChain(ch);
		}
	}
	
	removeReferences();
}

void Model::housekeeping()
{
	for (Molecule &mc : _molecules)
	{
		std::string ch = mc.chain_id();
		_chain2Molecule[ch] = &mc;
	}

	if (_chain2Molecule.size() < _chain2Entity.size())
	{
		return;
	}

	createMolecules();
}

void Model::extractTorsions()
{
	for (Molecule &mc : _molecules)
	{
		mc.extractTorsionAngles(_currentAtoms);
	}
}

void Model::loadAndRefine()
{
	File *f = File::loadUnknown(_filename);
	AtomContent *contents = f->atoms();
	contents->setResponder(this);
	contents->alignAnchor();
	contents->refinePositions();
	
	_currentFile = f;
	_currentAtoms = contents;
}

void Model::finishedRefinement()
{
	extractTorsions();
	removeReferences();

	if (_responder)
	{
		_responder->modelReady();
	}
}
