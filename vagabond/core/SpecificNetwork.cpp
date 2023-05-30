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

#include "SpecificNetwork.h"
#include "Network.h"
#include "Instance.h"
#include "NetworkBasis.h"
#include "Model.h"
#include "BondSequence.h"

SpecificNetwork::SpecificNetwork(Network *network, Instance *inst) :
StructureModification(inst, 120, network->dims())
{
	_torsionType = TorsionBasis::TypeOnPath;

	_instance = inst;
	_network = network;

	setup();
}

void SpecificNetwork::setup()
{
	startCalculator();
	processCalculatorDetails();
}

void SpecificNetwork::completeAtomMaps(CalcDetails &cd)
{
	std::vector<Instance *> others = _network->instances();
	
	for (Instance *other : others)
	{
		int idx = _network->indexForInstance(other);
		other->model()->load(Model::NoGeometry);
		
		AtomGroup *grp = other->currentAtoms();
		
		for (AtomMapping &am : cd.atoms)
		{
			Atom *search = am.atom;
			if (!search)
			{
				continue;
			}

			Atom *equiv = other->equivalentForAtom(_instance, search);
			
			if (equiv)
			{
				glm::vec3 pos = equiv->initialPosition();
				am.mapping->alter_value(idx, pos);
			}
		}
		
		if (other != _instance)
		{
			other->model()->unload();
		}
	}
}

void SpecificNetwork::completeMap(TorsionMapping &map)
{
	const ResidueId id = map.param->residueId();
	Residue *master_res = _instance->equivalentMaster(id);

	std::vector<Instance *> others = _network->instances();

	for (Instance *other : others)
	{
		Residue *other_res = other->equivalentLocal(master_res);
		if (!other_res)
		{
			continue;
		}
		
		int idx = _network->indexForInstance(other);
		map.mapping->alter_value(idx, map.param->value());
	}
}

void SpecificNetwork::getAtomDetails(BondSequence *seq, CalcDetails &cd)
{
	const std::vector<AtomBlock> &blocks = seq->blocks();
	
	for (int i = 0; i < blocks.size(); i++)
	{
		AtomMapping am{};
		am.atom = blocks[i].atom;
		
		if (am.atom != nullptr)
		{
			am.mapping = _network->blueprint_vec3_copy();
		}
		
		cd.atoms.push_back(am);
	}
}

void SpecificNetwork::getTorsionDetails(TorsionBasis *tb, CalcDetails &cd)
{
	for (size_t i = 0; i < tb->parameterCount(); i++)
	{
		Parameter *pm = tb->parameter(i);
		TorsionMapping tm{};
		tm.param = pm;
		if (pm && !pm->isConstrained())
		{
			tm.mapping = _network->blueprint_scalar_copy();
			completeMap(tm);
		}
		
		cd.torsions.push_back(tm);
	}
}

void SpecificNetwork::getDetails(BondCalculator *bc)
{
	CalcDetails cd{};

	BondSequence *seq = bc->sequence();
	getAtomDetails(seq, cd);
	completeAtomMaps(cd);

	TorsionBasis *tb = bc->torsionBasis();
	getTorsionDetails(tb, cd);
	
	_calcDetails[bc] = cd;
}

void SpecificNetwork::processCalculatorDetails()
{
	for (BondCalculator *calc : _calculators)
	{
		getDetails(calc);
	}
}

void SpecificNetwork::torsionBasisMods(TorsionBasis *tb)
{
	NetworkBasis *nb = static_cast<NetworkBasis *>(tb);
	nb->setSpecificNetwork(this);
}

glm::vec3 SpecificNetwork::positionForIndex(BondCalculator *bc,
                                            int idx, float *vec, int n) const
{
	std::vector<float> vals(n);
	for (int i = 0; i < n; i++)
	{
		vals[i] = vec[i];
	}

	const CalcDetails &cd = _calcDetails.at(bc);
	Mapped<glm::vec3> *const map = cd.atoms.at(idx).mapping;
	glm::vec3 pos = map->interpolate_variable(vals);
	return pos;
}
