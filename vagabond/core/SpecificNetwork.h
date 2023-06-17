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
#include "PositionSampler.h"
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
	SpecificNetwork(Network *network, Instance *inst);
	
	Instance *instance()
	{
		return _instance;
	}
	
	Mapped<float> *mapForParameter(Parameter *p) const
	{
		if (_param2Map.count(p) == 0)
		{
			return nullptr;
		}
		return _param2Map.at(p);
	}

	float torsionForIndex(BondSequence *seq,
	                      int idx, const float *vec) const;
	
	bool valid_position(const std::vector<float> &vals);

	virtual void torsionBasisMods(TorsionBasis *tb);

	int submitJob(bool show, const std::vector<float> vals);

	virtual void prewarnAngles(BondSequence *seq, 
	                           const std::vector<float> &vals,
	                           std::vector<float> &angles);
	virtual bool prewarnAtoms(BondSequence *bc, const std::vector<float> &vals,
	                  Vec3s &positions);
	
	int detailsForParam(Parameter *parameter, BondCalculator **calc);
	void zeroVertices();
	void setup();

	int splitFace(Parameter *parameter, int tidx);
	
	void setDisplayInterval(int interval)
	{
		_displayInterval = interval;
	}
protected:
	virtual bool handleAtomMap(AtomPosMap &aps);
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

	void getTorsionDetails(TorsionBasis *tb, CalcDetails &cd);

	void prepareAtomMaps(BondSequence *seq, PosMapping *pm);
	void completeTorsionMap(TorsionMapping &map);
	void prewarnParameters(BondSequence *bc, const std::vector<float> &vals);
	
	struct PrewarnResults
	{
		Vec3s positions;
		std::vector<float> torsions;
		bool acceptable;
	};

	std::map<BondCalculator *, CalcDetails> _calcDetails;
	std::map<BondCalculator *, PosMapping *> _atomDetails;
	std::map<BondSequence *, PrewarnResults> _prewarnedResults;
	std::map<Parameter *, Mapped<float> *> _param2Map;
	int _jobNum = 0;
	int _display = 0;
	int _displayInterval = 100;
	std::atomic<bool> _writing{false};
};

#endif
