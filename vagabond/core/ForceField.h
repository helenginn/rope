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

class ForceField
{
public:
	ForceField(AtomGroup *grp);
	
	enum Template
	{
		CAlphaSeparation,
	};

	void setTemplate(Template t)
	{
		_t = t;
	}

	void setup();

	struct Restraint
	{
		enum Type
		{
			Spring,
			Angle,
			VdW,
		};

		Type type;
		float target = 0;
		float current = 0;
		float deviation = 1;
		
		Atom *atoms[3] = {nullptr, nullptr, nullptr};
		
		glm::vec3 a{};
		glm::vec3 b{};
		glm::vec3 c{};
		
		Restraint(Type t, float targ, float dev)
		{
			type = t;
			target = targ;
			deviation = dev;
		}
		
		void terminalAtoms(Atom **start, Atom **end) const
		{
			*start = atoms[0];
			
			if (type == Angle)
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

	void updateTargets(AtomPosMap &aps, MechanicalBasis *mb);
	double score(AtomPosMap &aps);

	const std::list<Restraint> &restraints() const
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

private:
	void setupContributions(MechanicalBasis *mb);
	void setupCAlphaSeparation();
	void updateRestraint(Restraint &r, AtomPosMap &aps);
	float gradientForRestraint(const Restraint &r);
	
	float contributionForRestraint(const Restraint &r, glm::vec3 start,
	                               glm::vec3 end);

	AtomGroup *_group = nullptr;

	std::list<Restraint> _restraints;
	PCA::Matrix _targets{};
	PCA::Matrix _weights{};
	
	PCA::SVD _contributions{};
	PCA::Matrix _validity{};
	Template _t;
};

#endif
