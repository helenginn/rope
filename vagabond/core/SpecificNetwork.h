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
#include <list>

template <typename Type> class Mapped;

class Instance;
class Network;
class TorsionBasis;
class BondSequence;

class SpecificNetwork : public StructureModification, public PositionSampler
{
public:
	SpecificNetwork(Network *network, Instance *inst);
	
	virtual glm::vec3 positionForIndex(BondCalculator *bc,
	                                   int idx, float *vec, int n) const;

	virtual void torsionBasisMods(TorsionBasis *tb);
	void setup();
private:
	void getDetails(BondCalculator *bc);
	void processCalculatorDetails();

	Instance *_instance = nullptr;
	Network *_network = nullptr;

	struct AtomMapping
	{
		Atom *atom;
		Mapped<glm::vec3> *mapping;
	};

	struct TorsionMapping
	{
		Parameter *param;
		Mapped<float> *mapping;
	};
	
	struct CalcDetails
	{
		std::vector<AtomMapping> atoms;
		std::vector<TorsionMapping> torsions;
	};
	
	void getAtomDetails(BondSequence *seq, CalcDetails &cd);
	void getTorsionDetails(TorsionBasis *tb, CalcDetails &cd);

	void completeAtomMaps(CalcDetails &cd);
	void completeMap(TorsionMapping &map);

	std::map<BondCalculator *, CalcDetails> _calcDetails;
};

#endif
