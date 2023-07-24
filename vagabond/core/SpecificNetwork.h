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

#ifndef __vagabond__SpecificNetwork__
#define __vagabond__SpecificNetwork__

#include "StructureModification.h"
#include "Instance.h"
#include "Model.h"
#include "PositionSampler.h"
#include "Network.h"
#include <vagabond/utils/Vec3s.h>
#include <vagabond/core/Responder.h>
#include <list>

template <typename Type> class Mapped;

class Instance;
class Network;
class TorsionBasis;
class BondSequence;

class SpecificNetwork : public StructureModification, public PositionSampler,
public HasResponder<Responder<SpecificNetwork>>
{
public:
	SpecificNetwork(json &j);
	SpecificNetwork(Network *network, Instance *inst);
	
	Instance *instance()
	{
		return _instance;
	}
	
	Network *network()
	{
		return _network;
	}

	nlohmann::json jsonForBondMap(int idx) const;
	void setJsonForParameter(Parameter *p, const nlohmann::json &j);
	
	bool hasBondMappingFor(const Parameter *p) const
	{
		return (_param2BondMap.count(p) > 0);
	}
	
	std::pair<Mapped<Floats> *, int> bondMappingFor(const Parameter *p) const
	{
		if (_param2BondMap.count(p) == 0)
		{
			return std::make_pair(nullptr, -1);
		}
		std::pair<Mapped<Floats> *, int> ret;
		ret.first = _bondMaps[_param2BondMap.at(p).first];
		ret.second = _param2BondMap.at(p).second;
		return ret;
	}
	
	const std::set<Parameter *> &parameters() const
	{
		return _parameters;
	}
	
	Coord::Interpolate<float> torsion(BondSequence *seq, int idx,
	                                  const Coord::Get &coord) const;
	
	bool valid_position(const std::vector<float> &vals);

	virtual void torsionBasisMods(TorsionBasis *tb);

	int submitJob(bool show, const std::vector<float> vals, int save_id = -1);

	virtual bool prewarnAtoms(BondSequence *bc, const Coord::Get &get,
	                          Vec3s &positions);
	void prewarnBonds(BondSequence *seq, const Coord::Get &get, Floats &torsions);
	
	void zeroVertices();
	void setup();
	
	void setDisplayInterval(int interval)
	{
		_displayInterval = interval;
	}

	friend inline void to_json(json &j, const SpecificNetwork &sn);
	friend inline void from_json(const json &j, SpecificNetwork &sn);
protected:
	virtual bool handleAtomMap(AtomPosMap &aps);
	virtual bool handleAtomList(AtomPosList &apl);
private:
	void getDetails(BondCalculator *bc);
	void customModifications(BondCalculator *calc, bool has_mol);
	void processCalculatorDetails();
	void updateAtomsFromDerived(int idx);

	Instance *_instance = nullptr;
	Network *_network = nullptr;

	struct TorsionMapping
	{
		Parameter *param;
		Mapped<float> *mapping;
	};
	
	struct CalcDetails
	{
		std::vector<TorsionMapping> torsions;
		
		int index_for_param(Parameter *p)
		{
			for (size_t i = 0; i < torsions.size(); i++)
			{
				if (torsions[i].param == p) return i;
			}

			return -1;
		}
	};
	
	typedef Mapped<Vec3s> PosMapping;
	typedef Mapped<Floats> BondMapping;

	void grabParamMaps(json &json);

	void prepareAtomMaps(BondSequence *seq, PosMapping *pm);
	void prepareBondMaps(BondSequence *seq, BondMapping *pm);
	
	struct PrewarnResults
	{
		Vec3s positions;
		std::vector<float> torsions;
		bool acceptable;
	};

	std::map<BondCalculator *, PosMapping *> _atomDetails;
	std::map<BondCalculator *, BondMapping *> _bondDetails;

	std::map<const Parameter *, std::pair<int, int>> _param2BondMap;
	std::vector<BondMapping *> _bondMaps;
	std::set<Parameter *> _parameters;

	int _jobNum = 0;
	int _display = 0;
	int _displayInterval = 100;
	bool _fromJson = false;
	std::atomic<bool> _writing{false};
};

inline void to_json(json &j, const SpecificNetwork &sn)
{
	j["model"] = sn._instance->model()->id();
	j["main_instance"] = sn._instance->id();
	sn._network->add_info(j);

	for (int i = 0; i < sn._bondMaps.size(); i++)
	{
		j["bondmaps"][i] = sn.jsonForBondMap(i);
	}

	for (auto it = sn._param2BondMap.begin(); it != sn._param2BondMap.end(); it++)
	{
		nlohmann::json param;
		param["param"]["res"] = it->first->residueId();
		param["param"]["desc"] = it->first->desc();
		param["param"]["map_idx"] = it->second.first;
		param["param"]["bond_idx"] = it->second.second;

		j["params"].push_back(param);
	}
}

#endif
