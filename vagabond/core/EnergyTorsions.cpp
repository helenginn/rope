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

#include "PairwiseDeviations.h"
#include "EnergyTorsions.h"
#include "BondSequence.h"
#include "RTMotion.h"
#include "ParamSet.h"
#include "engine/Task.h"
#include "LoopRoundResidues.h"
#include <vagabond/utils/OpSet.h>
#include <gemmi/elem.hpp>

using Torsion2Energy = EnergyTorsions::Torsion2Energy;

EnergyTorsions::EnergyTorsions(BondSequence *sequence, const RTMotion &motions)
{
	prepare(sequence, motions);
}

EnergyTorsions::~EnergyTorsions()
{

}

auto energy_term(EnergyTorsions *et, const std::set<ResidueId> &forResidues)
{
	LoopMechanism loop = loop_mechanism(et->pairs(), et->perResiduePairs(),
	                                    forResidues);

	return [loop, et]
	(BondSequence *seq) -> ActivationEnergy
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		const std::vector<Torsion2Energy> &energies = et->energies();
		
		float total = 0; // already in kJ/mol
		auto get_energies = [&total, &blocks, &energies]
		(const std::vector<int> &torsions)
		{
			for (const int &idx : torsions)
			{
				float t = blocks[idx].torsion;
				float energy = energies[idx](t);
				total += energy;
			}
		};
		
		loop(get_energies);
		
		return {(float)total};
	};
}

Task<BondSequence *, ActivationEnergy> *
EnergyTorsions::energy_task(const std::set<ResidueId> &forResidues)
{
	auto energy = energy_term(this, forResidues);
	auto *task = new Task<BondSequence *, ActivationEnergy>(energy);
	return task;

}

auto bell_curve(float z, float rel_angle, float breadth)
{
	return [z, rel_angle, breadth](float f)
	{
		if (f != f || !std::isfinite(f))
		{
			return 0.f;
		}

		while (f < rel_angle - 180.f) f += 360.f;
		while (f >= rel_angle + 180.f) f -= 360.f;

		float root = (f - rel_angle) / breadth;
		float cbroot = 1 + root * root;
		float full = z / (cbroot * cbroot * cbroot);
		return full;

	};

}

EnergyTorsions::Torsion2Energy function_for_block(TorsionBasis *basis,
                                                  AtomBlock &block)
{
	using Torsion2Energy = EnergyTorsions::Torsion2Energy;
	if (block.torsion_idx < 0)
	{
		return {};
	}

	Parameter *p = basis->parameter(block.torsion_idx);
	if (p->isConstrained())
	{
		return {};
	}

	if (p->isPeptideBond())
	{
		float z = sqrt(72); // CA-N-C-CA: 6^2 + 6^2
		Torsion2Energy bellcurve = {bell_curve(-z, 180.f, 30)};
		return bellcurve;
	}

	ParamSet params(p);
	params.expandToSisters();
	
	float refTorsion = p->value();

	typedef std::map<Parameter *, float> ParamAngles;
	ParamAngles pAngles;
	
	struct Obstruction
	{
		float z;
		float rel_angle;
		float breadth;
	};

	// maximum 3 sisters on one side, 3 sisters on the other = 9 total

	std::vector<Obstruction> obs;
	obs.reserve(params.size());
	for (Parameter *q : params)
	{
		float qTorsion = q->value();

		gemmi::Element pEle = gemmi::Element(q->atom(0)->elementSymbol());
		gemmi::Element qEle = gemmi::Element(q->atom(3)->elementSymbol());
		int pz = pEle.atomic_number();
		int qz = qEle.atomic_number();
		float z = sqrt(pz * pz + qz * qz);
		float rel_angle = qTorsion - refTorsion;
		if (rel_angle > 10e10)
		{
			std::cout << "Warning: " << q->desc() << " has rel angle " << rel_angle << std::endl;
			std::cout << qTorsion << " " << refTorsion << std::endl;
		}
		
		obs.push_back({z, rel_angle, 60});
		if (pz > 1 && qz > 1)
		{
//			obs.push_back({z, rel_angle, 120});
		}
	}

	std::vector<Torsion2Energy> curves;
	for (Obstruction &ob : obs)
	{
		Torsion2Energy bellcurve = {bell_curve(ob.z, ob.rel_angle, ob.breadth)};
		curves.push_back(bellcurve);
	}

	auto calculate_contribution = [curves](const float &f)
	{
		float sum = 0;
		
		for (const Torsion2Energy &contribution : curves) 
		{
			sum += contribution(f);
		}

		return sum;
	};
	
	return calculate_contribution;
}

float weightForResidue(const RTMotion &motions, const ResidueId &res)
{
	std::vector<ResidueTorsion> rts = motions.headers_only();
	float biggest = 0;
	int i = -1;
	for (ResidueTorsion rt : rts)
	{
		i++;

		if (rt.local_id() != res)
		{
			continue;
		}
		
		const Motion &mot = motions.storage(i);
		float angle = fabs(mot.workingAngle());
		if (angle > biggest)
		{
			biggest = angle;
		}
	}

	// sigmoidal curve
	float r = 30.;
	biggest /= r;
	biggest *= biggest; // to the power 2
	biggest *= biggest; // to the power 4
	biggest *= biggest; // to the power 8
	return biggest / (biggest + 1);
}

void EnergyTorsions::prepare(BondSequence *sequence, 
                             const RTMotion &motions)
{
	TorsionBasis *basis = sequence->torsionBasis();
	for (AtomBlock &block : sequence->blocks())
	{
		Torsion2Energy func = function_for_block(basis, block);
		float weight = 1;
		
		if (func)
		{
			Parameter *p = basis->parameter(block.torsion_idx);
			ResidueId resi = p->residueId();
			_perResidues[resi].push_back(_energies.size());
		}
		else
		{
			func = [](const float &) { return 0.f; };
		}
		
		_pairs.push_back(_energies.size());
		_energies.push_back(func);
		_dampeners.push_back(weight);
	}
}
