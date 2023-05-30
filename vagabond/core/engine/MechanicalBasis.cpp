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

#define MAX_ANY_ANGLE (2.)

#define _USE_MATH_DEFINES
#include <math.h>
#include "engine/MechanicalBasis.h"
#include "BondTorsion.h"
#include "engine/ForceField.h"
#include <iostream>

MechanicalBasis::MechanicalBasis()
{

}

float MechanicalBasis::parameterForVector(BondCalculator *calculator,
                                          int idx, const float *vec, int n)
{
	if (idx >= 0 && idx < _mechAngles.size())
	{
		if (n > 0)
		{
			float change = _mechAngles[idx].change * vec[0];
			change *= _mechAngles[idx].mask ? 1 : 0;
			return _mechAngles[idx].start + change;
		}

		return _mechAngles[idx].start;
	}
	else
	{
		return 0;
	}
}

void MechanicalBasis::supplyDistances(PCA::Matrix &m, std::vector<Atom *> atoms)
{
	_distances = m;
	_atoms = atoms;
	_atomIdxs.clear();
	
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		_atomIdxs[_atoms[i]] = i;
	}
}

bool MechanicalBasis::doesTorsionAffectRestraint(const ForceField::Restraint &r, 
                                                 const Parameter *p)
{
	if (!p->isTorsion())
	{
		return true;
	}
	
	const BondTorsion *t = static_cast<const BondTorsion *>(p);

	Atom *a = nullptr;
	Atom *b = nullptr;
	r.terminalAtoms(&a, &b);
	
	int ra1 = _atomIdxs.count(a) ? _atomIdxs.at(a) : -1;
	int ra2 = _atomIdxs.count(b) ? _atomIdxs.at(b) : -1;
	int ta1 = _atomIdxs.count(t->atom(1)) ? _atomIdxs.at(t->atom(1)) : -1;
	int ta2 = _atomIdxs.count(t->atom(2)) ? _atomIdxs.at(t->atom(2)) : -1;

	if (ra1 < 0 || ra2 < 0 || ta1 < 0 || ta2 < 0)
	{
		return false;
	}
	
	int dist_between_targets = _distances[ra1][ra2];
	int first_leg = _distances[ta1][ra1];
	int second_leg = _distances[ta1][ra2];
	
	return (dist_between_targets - first_leg - second_leg == 0);
}

void MechanicalBasis::prepare(int dims)
{
	_angles.clear();
	_mechAngles.clear();

	for (size_t i = 0; i < _params.size(); i++)
	{
		float start = _params[i]->value();
		bool mask = !_params[i]->isConstrained();
		TorsionAngle ta = {start, mask};
		_angles.push_back(ta);

		MechAngle ma = {start, 0, 0, 0, mask};
		_mechAngles.push_back(ma);
	}
}

void MechanicalBasis::checkForceField(AtomPosMap &aps)
{
	_forceField->updateTargets(aps, this);
	
	for (size_t i = 0; (i < _forceField->weightCount() &&
	                    i < parameterCount()); i++)
	{
		_mechAngles[i].acceleration = _forceField->weight(i);
	}
}

void MechanicalBasis::refreshMechanics(AtomPosMap &aps)
{
	checkForceField(aps);
}

void MechanicalBasis::prepareRecalculation()
{
	if (!_run)
	{
		assignStartingVelocities();
		_run = false;
	}
	
	incrementTime();
}

void MechanicalBasis::incrementTime()
{
	double sum = 0;
	double max = 0;

	for (size_t i = 0; i < _mechAngles.size(); i++)
	{
		_mechAngles[i].velocity += _mechAngles[i].acceleration;
		_mechAngles[i].acceleration = 0;
		float mag = fabs(_mechAngles[i].velocity);
		
		sum += mag;
		if (mag > max)
		{
			max = mag;
		}
	}

	sum /= (float)_mechAngles.size();
	double ratio = 1 / sum;

	max *= ratio;
	
	if (max > MAX_ANY_ANGLE)
	{
		double correct = max / MAX_ANY_ANGLE;
		ratio /= correct;
		sum /= correct;
	}
	
	ratio = deg2rad(ratio);
	ratio /= 100;
	
//	std::cout << "Timepoint add: " << ratio << " vagaseconds" << std::endl;

	for (size_t i = 0; i < _mechAngles.size(); i++)
	{
		_mechAngles[i].change += _mechAngles[i].velocity * ratio;
	}
}

void MechanicalBasis::assignStartingVelocities()
{
	for (size_t i = 0; i < _mechAngles.size(); i++)
	{
		float rnd = (rand() / (double)RAND_MAX) - 0.5;
		_mechAngles[i].velocity = rnd;
	}

}
