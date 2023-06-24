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
	_threads = 2;
	_instance->load();
	_instance->currentAtoms()->recalculate();
	
	grabParamMaps(j);
}

void SpecificNetwork::grabParamMaps(json &j)
{
	AtomGroup *atoms = _instance->currentAtoms();
	typedef Mapping<NETWORK_DIMS, float> Concrete;

	for (json &entry : j["maps"])
	{
		ResidueId id = entry["param"]["res"];
		std::string desc = entry["param"]["desc"];
		Parameter *param = _instance->currentAtoms()->findParameter(desc, id);
		
		if (!param)
		{
			throw std::runtime_error("Could not find vital parameter " + 
			                         id.as_string() + " " + desc);
		}

		json &tmp = entry["map"];
		_param2Map[param] = new Concrete();
		setJsonForParameter(param, tmp);
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

	for (BondCalculator *calc : _calculators)
	{
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

void SpecificNetwork::completeTorsionMap(TorsionMapping &map)
{
	if (_param2Map.count(map.param) > 0)
	{
		map.mapping = _param2Map[map.param];
		return;
//		return;
	}

	const ResidueId id = map.param->residueId();
	Residue *master_res = _instance->equivalentMaster(id);
	_param2Map[map.param] = map.mapping;

	std::vector<Instance *> others = _network->instances();

	for (Instance *other : others)
	{
		Residue *other_res = other->equivalentLocal(master_res);
		if (!other_res)
		{
			continue;
		}
		
		int idx = _network->indexForInstance(other);
		float val = map.param->value();

		TorsionRef ref = other_res->copyTorsionRef(map.param->desc());
		if (ref.valid())
		{
			float tmp = ref.refinedAngle();
			matchDegree(val, tmp);
			val = tmp;
		}

		map.mapping->alter_value(idx, val);
	}
	
	map.mapping->update();
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
			completeTorsionMap(tm);
		}
		
		cd.torsions.push_back(tm);
	}
}

void SpecificNetwork::getDetails(BondCalculator *bc)
{
	CalcDetails cd{};

	BondSequence *seq = bc->sequence();
	PosMapping *pm = _network->blueprint_vec3s_copy();

	prepareAtomMaps(seq, pm);

	TorsionBasis *tb = bc->torsionBasis();
	getTorsionDetails(tb, cd);
	
	_calcDetails[bc] = cd;
	_atomDetails[bc] = pm;
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

bool SpecificNetwork::prewarnAtoms(BondSequence *seq, 
                                   const std::vector<float> &vals,
                                   Vec3s &positions)
{
	bool accept = false;
	BondCalculator *bc = seq->calculator();
	PosMapping *pm = _atomDetails[bc];
	positions = pm->interpolate_variable(vals, &accept);
	
	return accept;
}

Coord::NeedsUpdate SpecificNetwork::needsUpdate(BondSequence *seq, int idx,
                                         const Coord::Get &coord)
{
	Coord::NeedsUpdate definitely = [](const Coord::Get &coord) { return true; };

	std::vector<float> vals(_network->dims());
	for (int i = 0; i < _network->dims(); i++)
	{
		vals[i] = coord(i);
	}

	BondCalculator *bc = seq->calculator();
	const CalcDetails &cd = _calcDetails.at(bc);

	const TorsionMapping &tm = cd.torsions.at(idx);
	Mapped<float> *const map = tm.mapping;
	
	if (!map)
	{
		return definitely;
	}

	int fidx = map->face_idx_for_point(vals);
	
	if (fidx < 0)
	{
		return definitely;
	}
	
	Mappable<float> *face = map->face_for_index(fidx);
	int version = map->face_for_index(fidx)->version();
	int pc = map->point_count_for_face(fidx);
	Coord::NeedsUpdate update;
	update = [face, version, pc](const Coord::Get &coord) -> bool
	{
		if (!face || !face->valid() || face->version() != version)
		{
			return true;
		}

		std::vector<float> weights(pc);
		face->point_to_barycentric(coord, weights);
		for (float &f : weights)
		{
			if (f < -1e-6 || f > 1+1e-6)
			{
				return true;
			}
		}
		
		return false;
	};
	
	return update;
}

Coord::Interpolate<float> SpecificNetwork::torsion(BondSequence *seq, int idx, 
                                                   const Coord::Get &coord) const
{
	std::vector<float> vals(_network->dims());
	for (int i = 0; i < _network->dims(); i++)
	{
		vals[i] = coord(i);
	}

	BondCalculator *bc = seq->calculator();
	const CalcDetails &cd = _calcDetails.at(bc);

	const TorsionMapping &tm = cd.torsions.at(idx);
	Mapped<float> *const map = tm.mapping;
	if (!map)
	{
		return [](const Coord::Get &) { return 0;};
	}

	Coord::Interpolate<float> interpolate = map->interpolate_function(coord);
	
	return interpolate;
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

int SpecificNetwork::detailsForParam(Parameter *parameter, BondCalculator **calc)
{
	*calc = nullptr;

	for (BondCalculator *bc : _calculators)
	{
		CalcDetails &cd = _calcDetails[bc];
		int idx = cd.index_for_param(parameter);
		
		if (idx >= 0)
		{
			*calc = bc;
			return idx;
		}
	}

	return -1;
}

int SpecificNetwork::pointCount(Parameter *parameter)
{
	for (BondCalculator *bc : _calculators)
	{
		CalcDetails &cd = _calcDetails[bc];
		int idx = cd.index_for_param(parameter);
		if (idx < 0)
		{
			continue;
		}

		Mapped<float> *map = cd.torsions[idx].mapping;
		return map->pointCount();
	}
	
	return -1;
}

int SpecificNetwork::splitFace(Parameter *parameter, 
                               const std::vector<float> &point)
{
	for (BondCalculator *bc : _calculators)
	{
		CalcDetails &cd = _calcDetails[bc];
		int idx = cd.index_for_param(parameter);

		if (idx < 0)
		{
			continue;
		}
		
		Mapped<float> *map = cd.torsions[idx].mapping;
		float value = map->interpolate_variable(point);
		int new_idx = map->add_point(point);
		map->alter_value(new_idx, value);
		map->crack_existing_face(new_idx);
		map->delaunay_refine();
		refresh(parameter);

		return new_idx;
	}

	return -1;
}

void SpecificNetwork::refresh(Parameter *p)
{
	json info = jsonForParameter(p);
	setJsonForParameter(p, info);
}

void SpecificNetwork::setJsonForParameter(Parameter *p, const json &j)
{
	typedef Mapping<NETWORK_DIMS, float> Concrete;
	Mapped<float> *tmp = mapForParameter(p);
	Concrete *map = static_cast<Concrete *>(tmp);

	map->invalidate();
	*map = j["map"];
}

json SpecificNetwork::jsonForParameter(Parameter *p) const
{
	typedef Mapping<NETWORK_DIMS, float> Concrete;
	Mapped<float> *tmp = mapForParameter(p);
	Concrete *map = static_cast<Concrete *>(tmp);

	json j;
	j["map"] = *map;
	return j;
}

