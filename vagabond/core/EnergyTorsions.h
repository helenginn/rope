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

#ifndef __vagabond__EnergyTorsions__
#define __vagabond__EnergyTorsions__

#include <functional>
#include <map>
#include <vector>
#include <set>
#include <cstddef>
#include <vagabond/utils/OpSet.h>
#include "PairwiseDeviations.h"

struct ActivationEnergy; 

template <typename In, typename Out> class Task;
class BondSequence;
class RTMotion;
struct ResidueId;

class EnergyTorsions
{
public:
	EnergyTorsions(BondSequence *sequence, const RTMotion &motions,
	               const std::function<int(Parameter *)> &lookup);

	~EnergyTorsions();
	
	typedef std::function<float(const float &)> Torsion2Energy;
	
	const std::map<ResidueId, std::vector<int>> &perResiduePairs()
	{
		return _perResidues;
	}

	const std::vector<Torsion2Energy> &energies()
	{
		return _energies;
	}

	const std::vector<int> &pairs() const
	{
		return _pairs;
	}
	
	float reference_torsion(int idx, float frac);

	Task<BondSequence *, ActivationEnergy> *
	energy_task(const std::set<ResidueId> &forResidues, float frac);
private:
	void prepare(BondSequence *sequence, const RTMotion &motions,
	             const std::function<int(Parameter *)> &lookup);

	std::vector<int> _pairs;
	std::map<ResidueId, std::vector<int>> _perResidues;
	std::vector<std::pair<float, float> > _angles;
	std::vector<Torsion2Energy> _energies;
	std::vector<OpSet<ResidueId>> _partitions;
};

#endif
