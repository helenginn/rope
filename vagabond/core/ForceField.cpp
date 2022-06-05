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

#include "BondTorsion.h"
#include "ForceField.h"
#include "MechanicalBasis.h"
#include "matrix_functions.h"

ForceField::ForceField(AtomGroup *grp)
{
	_group = grp;
}

void ForceField::setup()
{
}

void ForceField::updateRestraint(Restraint &r, AtomPosMap &aps)
{
	if (r.type == Restraint::Spring ||
	    r.type == Restraint::VdW)
	{
		if (aps.count(r.atoms[0]) == 0 || aps.count(r.atoms[1]) == 0)
		{
			r.current = 0;
			return;
		}

		r.a = aps.at(r.atoms[0]).samples[1];
		r.b = aps.at(r.atoms[1]).samples[1];

		float length = glm::length(r.a - r.b);
		r.current = length;
	}

}

float ForceField::gradientForRestraint(const Restraint &r)
{
	if (r.type == Restraint::Spring)
	{
		float diff = r.current - r.target;
		diff /= r.deviation;
		/* gradient of y = x^2 */
		
		return 2 * diff;
	}
	
	return 0;
}

float ForceField::contributionForRestraint(const Restraint &r, glm::vec3 start,
                                           glm::vec3 end)
{
	float con = 0;
	if (r.type == Restraint::Spring || r.type == Restraint::VdW)
	{
		con = bond_rotation_on_distance_gradient(start, end, r.a, r.b);
	}

	return con;
}

void ForceField::setupContributions(MechanicalBasis *mb)
{
	if (_contributions.u.rows > 0 &&
	    _contributions.u.cols > 0)
	{
		return;
	}
	
	/* allocate column vector of target gradients */
	setupMatrix(&_targets, _restraints.size(), 1);
	setupMatrix(&_weights, mb->torsionCount(), 1);

	setupSVD(&_contributions, mb->torsionCount(), _restraints.size());

	setupMatrix(&_validity, mb->torsionCount(), _restraints.size());
	
	for (size_t j = 0; j < mb->torsionCount(); j++)
	{
		int i = 0;
		for (Restraint &r : _restraints)
		{
			bool valid = mb->doesTorsionAffectRestraint(r, mb->torsion(j));
			_validity[j][i] = valid ? 1 : -1;
			i++;
		}
	}
}

void ForceField::updateTargets(AtomPosMap &aps, MechanicalBasis *mb)
{
	setupContributions(mb);
	zeroMatrix(&_targets);
	zeroMatrix(&_contributions.u);

	int idx = 0;
	for (Restraint &r : _restraints)
	{
		updateRestraint(r, aps);
		float val = gradientForRestraint(r);
		_targets[idx][0] = val;

		for (size_t i = 0; i < mb->torsionCount(); i++)
		{
			BondTorsion *t = mb->torsion(i);

			bool valid = _validity[i][idx] > 0;
			
			if (!valid)
			{
				continue;
			}

			glm::vec3 cPos = aps.at(t->atom(1)).samples[1];
			glm::vec3 dPos = aps.at(t->atom(2)).samples[1];

			float con = contributionForRestraint(r, cPos, dPos);
			_contributions.u[i][idx] = con;
		}

		idx++;
	}
	
	invertSVD(&_contributions);
	zeroMatrix(&_weights);
	
	for (size_t i = 0; i < _weights.rows; i++)
	{
		for (size_t j = 0; j < _contributions.u.cols; j++)
		{
			for (size_t i = 0; i < _contributions.u.rows; i++)
			{
				_weights[i][0] += _contributions.u[i][j] * _targets[j][0];
			}
		}
	}
}

