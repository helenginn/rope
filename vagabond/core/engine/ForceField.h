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

#ifndef __vagabond__ForceField__
#define __vagabond__ForceField__

#include <list>
#include <vagabond/utils/svd/PCA.h>
#include "Job.h"

class Atom;
class AtomGroup;
class MechanicalBasis;

#include "FFProperties.h"

class ForceField
{
public:
	ForceField(AtomGroup *grp);
	ForceField(FFProperties &props);
	
	~ForceField();

	void setTemplate(FFProperties::Template t)
	{
		_t = t;
	}
	
	void setJob(Job *j)
	{
		_job = j;
	}
	
	Job *job()
	{
		return _job;
	}

	void setup();

	struct Restraint
	{
		enum Type
		{
			Spring,
			HBond,
			VdW,
			Momentum,
			HardLimit,
		};

		Type type;
		float target = 0;
		float current = 0;
		float currsq = 0;
		float deviation = 1;
		float target_angle = 0;
		float current_angle = 0;
		float dev_angle = 0;
		
		Atom *atoms[3] = {nullptr, nullptr, nullptr};
		Atom *reporters[3] = {nullptr, nullptr, nullptr};
		
		glm::vec3 pos[3]{};
		
		Restraint(Type t, float targ, float dev)
		{
			type = t;
			target = targ;
			deviation = dev;
			
			if (type == HBond)
			{
				target = 3.1;
				deviation = 0.2;
				dev_angle = 10;
				target_angle = targ;
			}
		}
		
		void terminalAtoms(Atom **start, Atom **end) const
		{
			*start = atoms[0];
			
			if (type == HBond)
			{
				*end = atoms[2];
			}
			else
			{
				*end = atoms[1];
			}
		}
	};
	
	const size_t weightCount() const
	{
		return _weights.rows;
	}
	
	const float weight(int i)
	{
		return _weights[i][0];
	}
	
	void copyAtomMap(const AtomPosMap &aps)
	{
		_aps = aps;
	}

	void updateTargets(AtomPosMap &aps, MechanicalBasis *mb);
	
	void prepareCalculation();
	double score();
	void getColours(AtomPosMap &aps);

	const std::vector<Restraint> &restraints() const
	{
		return _restraints;
	}
	
	void addLength(Atom *a, Atom *b, float target, float dev)
	{
		Restraint r(Restraint::Spring, target, dev);
		r.atoms[0] = a; 
		r.atoms[1] = b;
		_restraints.push_back(r);
	}
	
	void processAtoms(Atom *a, Atom *b, Atom *report_a, Atom *report_b);

private:
	void setupContributions(MechanicalBasis *mb);
	void setupVanDerWaals();
	void setupCAlphaSeparation();
	void updateRestraint(Restraint &r);
	float gradientForRestraint(const Restraint &r);
	float valueForRestraint(const Restraint &r);
	void makeLookupTable();
	void assignToAtomGroup();
	void testHydrogenBond(Atom *a, Atom *b, Atom *report_a, Atom *report_b);
	
	float contributionForRestraint(const Restraint &r, glm::vec3 start,
	                               glm::vec3 end);


	bool isBackbone(Atom *a);
	
	struct LookupTable
	{
		Atom *a;
		size_t restraint_index;
		size_t atom_index;
	};
	
	std::vector<LookupTable> _table;
	std::map<Atom *, size_t> _tableIndices;
	std::map<Atom *, float> _tmpColours;

	AtomGroup *_group = nullptr;
	AtomGroup *_reporters = nullptr;

	AtomPosMap _aps;
	Job *_job = nullptr;
	std::vector<Restraint> _restraints;
	PCA::Matrix _targets{};
	PCA::Matrix _weights{};
	
	PCA::SVD _contributions{};
	PCA::Matrix _validity{};
	FFProperties::Template _t;
};

#endif
