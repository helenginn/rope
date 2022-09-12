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

#define _USE_MATH_DEFINES
#include <math.h>
#include "BondTorsion.h"
#include "ForceField.h"
#include "MechanicalBasis.h"
#include "ResidueId.h"
#include "AtomGroup.h"
#include "matrix_functions.h"

ForceField::ForceField(AtomGroup *grp)
{
	_group = grp;
	_reporters = new AtomGroup();
}

ForceField::ForceField(FFProperties &props)
{
	_group = props.group;
	_reporters = new AtomGroup();
	_t = props.t;
}

ForceField::~ForceField()
{
	_group->assignForceField(nullptr);

}

bool ForceField::isBackbone(Atom *a)
{
	return !a->hetatm();

	if (a->atomName() == "CA" || a->atomName() == "N" || a->atomName() == "C"
	    || a->atomName() == "O" || a->atomName() == "CB" || a->atomName() == "OXT")
	{
		return true;
	}
}

void ForceField::setupCAlphaSeparation()
{
	std::cout << "Making restraints for " << this << std::endl;
	AtomVector atoms;
	
	for (size_t i = 0; i < _group->size(); i++)
	{
		Atom *a = (*_group)[i];
		if (isBackbone(a))
		{
			atoms.push_back(a);
		}
	}

	int added = 0;
	const double comp = 15;
	
	std::map<Atom *, Atom *> tmpReporters;

	for (size_t i = 0; i < atoms.size(); i++)
	{
		Atom &ai = *atoms[i];
		Atom *cai = _group->atomByIdName(ResidueId(ai.residueNumber()), "CA");
		tmpReporters[atoms[i]] = cai;
		_reporters->add(cai);
	}

	for (size_t i = 0; i < atoms.size() - 1; i++)
	{
		Atom &ai = *atoms[i];
		Atom *cai = tmpReporters[&ai];

		for (size_t j = i + 1; j < atoms.size(); j++)
		{
			Atom &aj = *atoms[j];
			bool skip = false;
			
			for (size_t k = 0; k < ai.bondLengthCount(); k++)
			{
				if (ai.connectedAtom(k) == &aj)
				{
					skip = true;
				}
			}

			if (skip)
			{
				continue;
			}
			
			Atom *caj = tmpReporters[&aj];

			glm::vec3 diff = ai.initialPosition() - aj.initialPosition();
			double l = glm::dot(diff, diff);
			
			if (l < (comp * comp))
			{
				processAtoms(&ai, &aj, cai, caj);
				added++;
				
				if (added % 10000 == 0)
				{
					_restraints.reserve(_restraints.size() + 10000);
				}
			}
		}
	}
	
	std::cout << "Restraints: " << _restraints.size() << std::endl;
}

void ForceField::processAtoms(Atom *a, Atom *b, Atom *report_a, Atom *report_b)
{
	Restraint r(Restraint::VdW, 2.2, 0);
	r.atoms[0] = a;
	r.atoms[1] = b;
	r.reporters[0] = report_a;
	r.reporters[1] = report_b;
	_restraints.push_back(r);
	
	testHydrogenBond(a, b, report_a, report_b);
}

void ForceField::testHydrogenBond(Atom *a, Atom *b, Atom *report_a, Atom *report_b)
{
	Atom *atoms[] = {a, b};
	Atom *pre = nullptr;
	Atom *acceptor = nullptr;
	Atom *donor = nullptr;

	for (size_t i = 0; i < 2; i++)
	{
		if (atoms[i]->atomName() == "O")
		{
			acceptor = atoms[i];
		}

		if (atoms[i]->atomName() == "N")
		{
			donor = atoms[i];
		}
	}
	
	if (!(acceptor && donor))
	{
		return;
	}
	
	for (size_t i = 0; i < acceptor->bondLengthCount(); i++)
	{
		if (acceptor->connectedAtom(i)->atomName() == "C")
		{
			pre = acceptor->connectedAtom(i);
		}
	}
	
	if (!(pre))
	{
		return;
	}
	
	Restraint r(Restraint::HBond, 180, 15);
	r.atoms[0] = donor;
	r.atoms[1] = acceptor;
	r.atoms[2] = pre;
	
	for (size_t i = 0; i < 3; i++)
	{
		r.pos[i] = r.atoms[i]->initialPosition();
	}
	
	updateRestraint(r);
	
	if (fabs(r.current_angle - r.target_angle) > 30)
	{
		return;
	}
	
	if (fabs(r.current - r.target) > 0.5)
	{
		return;
	}
	
	std::cout << "Making hydrogen bond, " << pre->desc() << "-" 
	<< acceptor->desc() << "-H . . . " << donor->desc() << " (angle "
	<< r.current_angle << ", dist " << r.current << ")" << std::endl;

	r.reporters[0] = report_a;
	r.reporters[1] = report_b;
	_restraints.push_back(r);

}

void ForceField::setup()
{
	_restraints.clear();

	if (_t == FFProperties::CAlphaSeparation)
	{
		setupCAlphaSeparation();
	}
	
	makeLookupTable();
	
	assignToAtomGroup();
}

void ForceField::assignToAtomGroup()
{
	_group->assignForceField(this);
}

void ForceField::makeLookupTable()
{
	std::cout << "Making lookup table" << std::endl;
	_table.clear();
	_tableIndices.clear();
	
	std::map<Atom *, std::vector<LookupTable> > tmpMap;
	int count = 0;
	int reserve = _restraints.size();
	_table.reserve(reserve);

	for (size_t i = 0; i < _group->size(); i++)
	{
		Atom *a = (*_group)[i];
		bool first = true;
		
		for (size_t j = 0; j < _restraints.size(); j++)
		{
			Restraint &r = _restraints[j];
			for (size_t n = 0; n < 3; n++)
			{
				if (r.atoms[n] == a)
				{
					LookupTable lt{a, j, n};
					
					if (first)
					{
						_tableIndices[a] = _table.size();
					}

					_table.push_back(lt);
					count++;
					
					if (count >= reserve)
					{
						reserve += _restraints.size();
						_table.reserve(reserve);
					}
					
					first = false;
				}
			}
		}
	}
	std::cout << "... done." << std::endl;
}

void ForceField::updateRestraint(Restraint &r)
{
	r.current = NAN;
	r.current_angle = NAN;

	if (r.type == Restraint::Spring ||
	    r.type == Restraint::VdW ||
	    r.type == Restraint::HBond)
	{
		float length = glm::length(r.pos[0] - r.pos[1]);
		r.current = length;
	}
	
	if (r.type == Restraint::HBond)
	{
		glm::vec3 hbond =  (r.pos[1] - r.pos[0]);
		glm::vec3 accept = (r.pos[1] - r.pos[2]);
		hbond = glm::normalize(hbond);
		accept = glm::normalize(accept);

		r.current_angle = rad2deg(glm::angle(hbond, accept));
	}

}

float ForceField::valueForRestraint(const Restraint &r)
{
	if (r.current != r.current)
	{
		return 0;
	}

	double vdw = 0;
	double attract = 0;
	double penalty = 0;

	if (r.type == Restraint::VdW || r.type == Restraint::HBond)
	{
		float weight = 0.2;
		float ratio = r.target / r.current;
		float rat6 = ratio*ratio*ratio*ratio*ratio*ratio;
		float rat12 = rat6*rat6;
		vdw = weight * (rat12 - 2 * rat6);
	}

	if (r.type == Restraint::HBond)
	{
		attract = -20;
		float angle = fabs(r.target_angle - r.current_angle) / r.dev_angle;
		float dist = fabs(r.target - r.current) / r.deviation;
		angle *= angle;
		dist *= dist;

//		penalty += 1 / (1 + angle + dist) - 1;
	}
	
	return vdw + attract * penalty;
}

float ForceField::gradientForRestraint(const Restraint &r)
{
	if (r.current != r.current)
	{
		return NAN;
	}
	if (r.type == Restraint::Spring)
	{
		float diff = r.current - r.target;
		
		if (diff != diff)
		{
			return 0;
		}

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
		con = bond_rotation_on_distance_gradient(start, end, r.pos[0], r.pos[1]);
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

void ForceField::prepareCalculation()
{
	AtomPosMap::iterator it;
	int count = 0;
	
	for (it = _aps.begin(); it != _aps.end(); it++)
	{
		Atom *a = it->first;
		
		if (_tableIndices.count(a))
		{
			size_t next = _tableIndices[a];

			while (true)
			{
				LookupTable &lt = _table[next];
				if (lt.a != a)
				{
					break;
				}
				
				int ridx = lt.restraint_index;
				int aidx = lt.atom_index;
				
				_restraints[ridx].pos[aidx] = _aps.at(a).samples[1];
				count++;
				
				next++;
				
				if (next >= _table.size())
				{
					break;
				}
			}
		}
	}

	for (Restraint &r : _restraints)
	{
		updateRestraint(r);
	}
}

double ForceField::score()
{
	for (size_t i = 0; i < _group->size() && i < _reporters->size(); i++)
	{
		Atom *reporter = (*_reporters)[i];
		if (reporter)
		{
			_tmpColours[reporter] = 0.f;
		}
	}

	double sum = 0;
	double weights = _group->size();
	for (Restraint &r : _restraints)
	{
		double val = valueForRestraint(r);
		
		if (val != val)
		{
			continue;
		}
		
		for (size_t i = 0; i < 2; i++)
		{
			if (r.reporters[i] != nullptr)
			{
				_tmpColours[r.reporters[i]] += val / 10.f;
			}
		}

		sum += val;
	}

	for (size_t i = 0; i < _reporters->size(); i++)
	{
		double tmp = _tmpColours[(*_reporters)[i]];
		if (tmp > 1)
		{
			tmp = log(tmp) + 1;
		}
	}
	
	double ret = sum / weights;
	
	if (ret != ret)
	{
		ret = 0;
	}

	return ret;
}

void ForceField::getColours(AtomPosMap &aps)
{
	for (Atom *a : _reporters->atomVector())
	{
		if (aps.count(a))
		{
			float col = _tmpColours[a];
			aps[a].colour = col;
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
		updateRestraint(r);
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

