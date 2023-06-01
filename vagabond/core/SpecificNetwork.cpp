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
StructureModification(inst, 1, network->dims())
{
	_torsionType = TorsionBasis::TypeNetwork;
	_threads = 6;
	_mutex = new std::mutex();
	_posMutex = new std::mutex();
	_torsMutex = new std::mutex();

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
}

void SpecificNetwork::zeroVertices()
{
	std::vector<Instance *> others = _network->instances();
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

int SpecificNetwork::submitJob(bool show, std::vector<float> vals)
{
	Job job{};
	job.custom.allocate_vectors(1, _network->dims(), _num);
	job.pos_sampler = this;

	for (size_t i = 0; i < vals.size(); i++)
	{
		job.custom.vecs[0].mean[i] = vals[i];
	}

	job.requests = static_cast<JobType>(JobExtractPositions |
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
}

void SpecificNetwork::getTorsionDetails(TorsionBasis *tb, CalcDetails &cd)
{
	for (size_t i = 0; i < tb->parameterCount(); i++)
	{
		Parameter *pm = tb->parameter(i);
		TorsionMapping tm{};
		tm.param = pm;
		tm.mutex = new std::mutex();

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

void SpecificNetwork::prewarnAtoms(BondSequence *seq, 
                                   const std::vector<float> &vals)
{
	bool accept = false;
	BondCalculator *bc = seq->calculator();
	PosMapping *pm = _atomDetails[bc];
	std::unique_lock<std::mutex> plock(*_posMutex);
	Vec3s positions = pm->interpolate_variable(vals, &accept);
	plock.unlock();

	std::unique_lock<std::mutex> lock(*_mutex);
	_prewarnedResults[seq].positions = positions;
	_prewarnedResults[seq].acceptable = accept;
}

void SpecificNetwork::prewarnParameters(BondSequence *seq,
                                        const std::vector<float> &vals)
{
	BondCalculator *bc = seq->calculator();
	CalcDetails &cd = _calcDetails[bc];
	std::vector<float> angles;

	std::unique_lock<std::mutex> tlock(*_torsMutex);
	for (int idx = 0; idx < cd.torsions.size(); idx++)
	{
		Mapped<float> *const map = cd.torsions.at(idx).mapping;
		float angle = map->interpolate_variable(vals);
		angles.push_back(angle);
	}
	tlock.unlock();
	
	std::unique_lock<std::mutex> lock(*_mutex);
	_prewarnedResults[seq].torsions = angles;
}

void SpecificNetwork::prewarnPosition(BondSequence *seq, float *vec, int n)
{
	std::vector<float> vals(n);
	for (int i = 0; i < n; i++)
	{
		vals[i] = vec[i];
	}

	prewarnAtoms(seq, vals);
	prewarnParameters(seq, vals);
}

glm::vec3 SpecificNetwork::positionForIndex(BondSequence *seq, int idx) const
{
	std::unique_lock<std::mutex> lock(*_mutex);
	const PrewarnResults &ar = _prewarnedResults.at(seq);

	if (!ar.acceptable)
	{
		return glm::vec3(NAN, NAN, NAN);
	}

	return ar.positions.at(idx);
}

float SpecificNetwork::torsionForIndex(BondSequence *seq,
                                       int idx, const float *vec) const
{
	std::unique_lock<std::mutex> lock(*_mutex);
	const PrewarnResults &ar = _prewarnedResults.at(seq);
	
	return ar.torsions.at(idx);
}

void SpecificNetwork::customModifications(BondCalculator *calc, bool has_mol)
{
	calc->setPositionSampler(this);
}

bool SpecificNetwork::valid_position(const std::vector<float> &vals)
{
	return _network->valid_position(vals);
}

void SpecificNetwork::handleAtomMap(AtomPosMap &aps)
{
	sendResponse("atom_map", &aps);

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

