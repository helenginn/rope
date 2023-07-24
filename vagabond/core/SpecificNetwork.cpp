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
#include "EntityManager.h"
#include "Network.h"
#include "Instance.h"
#include "NetworkBasis.h"
#include "Model.h"
#include "BondSequence.h"
#include "ModelManager.h"
#include <vagabond/utils/Mapping.h>

SpecificNetwork::SpecificNetwork(json &j) :
StructureModification(j.at("model"), j.at("main_instance"), 1, 2)
{
	std::string ent = j["entity"];
	std::string modid = j["model"];
	std::string main_inst = j["main_instance"];

	Entity *entity = EntityManager::manager()->entity(ent);
	if (entity == nullptr)
	{
		throw std::runtime_error("Entity by name not found in this RoPE: " + ent);
	}

	Instance *main = ModelManager::manager()->instance(main_inst);
	if (main == nullptr)
	{
		throw std::runtime_error("Main instance " + main_inst + " not found.");
	}
	_instance = main;

	std::vector<Instance *> instances;
	for (std::string id : j["instances"])
	{
		Instance *instance = ModelManager::manager()->instance(id);
		if (instance == nullptr)
		{
			throw std::runtime_error("Instance " + id + " not found.");
		}
		instances.push_back(instance);
	}
	
	_network = new Network(entity, instances);
	_network->get_info(j);
	
	_torsionType = TorsionBasis::TypeNetwork;
	_threads = 1;
	_instance->load();
	_instance->currentAtoms()->recalculate();
	_fromJson = true;

	grabParamMaps(j);
	
	setup();
}

void SpecificNetwork::grabParamMaps(json &j)
{
	AtomGroup *atoms = _instance->currentAtoms();
	typedef Mapping<NETWORK_DIMS, float> Concrete;

	for (json &entry : j["params"])
	{
		ResidueId id = entry["param"]["res"];
		std::string desc = entry["param"]["desc"];
		Parameter *param = _instance->currentAtoms()->findParameter(desc, id);
		
		if (!param)
		{
			throw std::runtime_error("Could not find vital parameter " + 
			                         id.as_string() + " " + desc);
		}

		_parameters.insert(param);
		int map_idx = entry["param"]["map_idx"];
		int bond_idx = entry["param"]["bond_idx"];
		_param2BondMap[param] = std::make_pair(map_idx, bond_idx);
	}
	
	std::cout << "bond maps: " << j["bondmaps"].size() << std::endl;
	for (json &entry : j["bondmaps"])
	{
		typedef Mapping<NETWORK_DIMS, Floats> Concrete;
		Concrete *map = new Concrete(entry["map"]);
		_bondMaps.push_back(map);
	}
}

SpecificNetwork::SpecificNetwork(Network *network, Instance *inst) :
StructureModification(inst, 1, network->dims())
{
	_torsionType = TorsionBasis::TypeNetwork;
	_threads = 2;

	_instance = inst;
	_network = network;
	
	_instance->load();
	_instance->currentAtoms()->recalculate();

	setup();
}

void SpecificNetwork::setup()
{
	startCalculator();
	processCalculatorDetails();
	zeroVertices();
	_displayInterval = 100;
}

void SpecificNetwork::zeroVertices()
{
	std::vector<Instance *> others = _network->instances();
	_displayInterval = 1;

	for (Instance *other : others)
	{
		int idx = _network->indexForInstance(other);

		std::vector<float> cart = _network->pointForInstance(other);

		int num = submitJob(true, cart);
		retrieve();
		float dev = deviation(num);
		std::cout << other->id() << ": " << dev << ", ";

		updateAtomsFromDerived(idx);

		other->model()->load();

		if (other != _instance)
		{
			other->currentAtoms()->recalculate();
			other->superposeOn(_instance);
			float diff = other->currentAtoms()->residualAgainst("original");
			std::cout << diff << std::endl;
			other->model()->unload();
		}
	}
	
	triggerResponse();
}

void SpecificNetwork::updateAtomsFromDerived(int idx)
{
	for (BondCalculator *calc : _calculators)
	{
		BondSequence *seq = calc->sequence();
		PosMapping *pm = _atomDetails[calc];

		const std::vector<AtomBlock> &blocks = seq->blocks();
		Vec3s &list = pm->get_value(idx);

		for (int i = 0; i < blocks.size(); i++)
		{
			Atom *search = blocks[i].atom;
			if (!search)
			{
				continue;
			}

			glm::vec3 pos = search->derivedPosition();
			list[i] = pos;
		}
	}
}

int SpecificNetwork::submitJob(bool show, const std::vector<float> vals, 
                               int save_id)
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _network->dims(), _num);
		job.pos_sampler = this;

		for (size_t i = 0; i < vals.size(); i++)
		{
			job.custom.vecs[0].mean[i] = vals[i];
		}

		job.requests = static_cast<JobType>(JobPositionVector |
		                                    JobCalculateDeviations);
		if (!show)
		{
			job.requests = JobCalculateDeviations;
		}

		int t = calc->submitJob(job);
		_ticket2Point[t] = _jobNum;
	}

	_point2Score[_jobNum] = Score{};
	_jobNum++;
	return _jobNum - 1;
}

void SpecificNetwork::prepareAtomMaps(BondSequence *seq, PosMapping *pm)
{
	const std::vector<AtomBlock> &blocks = seq->blocks();
	std::vector<Instance *> others = _network->instances();
	
	for (Instance *other : others)
	{
		int idx = _network->indexForInstance(other);
		Vec3s &list = pm->get_value(idx);

		for (int i = 0; i < blocks.size(); i++)
		{
			Atom *search = blocks[i].atom;
			glm::vec3 pos = glm::vec3(NAN, NAN, NAN);
			
			if (search)
			{
				pos = search->derivedPosition();
			}

			list.push_back(pos);
		}
	}
}

void SpecificNetwork::prepareBondMaps(BondSequence *seq, BondMapping *pm)
{
	const std::vector<AtomBlock> &blocks = seq->blocks();
	TorsionBasis *basis = seq->torsionBasis();
	
	std::vector<Parameter *> params = basis->parameters();
	std::vector<Instance *> others = _network->instances();
	std::cout << "running prepareBondMaps" << std::endl;
	
	for (Instance *other : others)
	{
		int idx = _network->indexForInstance(other);
		Floats &list = pm->get_value(idx);
		Floats grad_zero;

		for (int i = 0; i < params.size(); i++)
		{
			const ResidueId id = params[i]->residueId();
			Residue *master_res = _instance->equivalentMaster(id);
			Residue *other_res = other->equivalentLocal(master_res);

			int idx = _network->indexForInstance(other);
			float val = params[i]->value();

			if (other_res)
			{
				std::cout << id << " " << params[i]->desc() << ": ";
				TorsionRef ref = other_res->copyTorsionRef(params[i]->desc());
				if (ref.valid())
				{
					float tmp = ref.refinedAngle();
					std::cout << val << " " << tmp;
					matchDegree(val, tmp);
					std::cout << " -> " << tmp << std::endl;
					val = tmp;
				}
			}

			_parameters.insert(params[i]);
			_param2BondMap[params[i]] = std::make_pair(_bondMaps.size(), i);
			list.push_back(val);
			grad_zero.push_back(0);
		}
		
		for (size_t d = 0; d < pm->n(); d++)
		{
			pm->set_gradients(idx, d, grad_zero);
		}
	}
}

void SpecificNetwork::getDetails(BondCalculator *bc)
{
	BondSequence *seq = bc->sequence();

	PosMapping *pm = _network->blueprint_vec3s_copy();
	prepareAtomMaps(seq, pm);
	_atomDetails[bc] = pm;
	
	if (!_fromJson)
	{
		BondMapping *bm = _network->blueprint_floats_copy();
		prepareBondMaps(seq, bm);
		_bondMaps.push_back(bm);
		_bondDetails[bc] = bm;
	}
}

void SpecificNetwork::processCalculatorDetails()
{
	int i = 0;
	std::cout << "bondmapsize " << _bondMaps.size() << std::endl;
	std::cout << "calcsize " << _calculators.size() << std::endl;
	for (BondCalculator *calc : _calculators)
	{
		getDetails(calc);

		if (_fromJson)
		{
			std::cout << "setting " << i << std::endl;
			std::cout << _bondMaps[i]->pointCount() << std::endl;
			_bondDetails[calc] = _bondMaps[i];
		}
		i++;
	}
}

void SpecificNetwork::torsionBasisMods(TorsionBasis *tb)
{
	NetworkBasis *nb = static_cast<NetworkBasis *>(tb);
	nb->setSpecificNetwork(this);
}

void SpecificNetwork::prewarnBonds(BondSequence *seq, const Coord::Get &get,
                                   Floats &torsions)
{
	BondCalculator *bc = seq->calculator();
	BondMapping *bm = _bondDetails[bc];
	torsions = bm->interpolate_position(get);
}

bool SpecificNetwork::prewarnAtoms(BondSequence *seq, 
                                   const Coord::Get &get,
                                   Vec3s &positions)
{
	bool accept = false;
	BondCalculator *bc = seq->calculator();
	PosMapping *pm = _atomDetails[bc];
	positions = pm->linear_value(get, &accept);
	
	return accept;
}

void SpecificNetwork::customModifications(BondCalculator *calc, bool has_mol)
{
	calc->setPositionSampler(this);
}

bool SpecificNetwork::valid_position(const std::vector<float> &vals)
{
	return _network->valid_position(vals);
}

bool SpecificNetwork::handleAtomList(AtomPosList &apl)
{
	sendResponse("atom_list", &apl);
	_display++;

	return (_display % _displayInterval) == 0;
}

bool SpecificNetwork::handleAtomMap(AtomPosMap &aps)
{
	sendResponse("atom_map", &aps);
	_display++;

	return (_display % _displayInterval) == 0;
}

void SpecificNetwork::setJsonForParameter(Parameter *p, const json &j)
{
	typedef Mapping<NETWORK_DIMS, float> Concrete;
//	Mapped<float> *tmp = mapForParameter(p);
//	Concrete *map = static_cast<Concrete *>(tmp);

//	*map = j["map"];
}

json SpecificNetwork::jsonForBondMap(int idx) const
{
	typedef Mapping<NETWORK_DIMS, Floats> Concrete;
	BondMapping *tmp = _bondMaps[idx];
	Concrete *conc = static_cast<Concrete *>(tmp);

	json j;
	j["map"] = *conc;
	return j;
}

