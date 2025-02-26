// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include <iostream>
#include <regex>
#include "matrix_functions.h"
#include "BondTorsion.h"
#include "AtomGroup.h"
#include "Atom.h"

double BondTorsion::_maxSeparation = 20;

BondTorsion::BondTorsion(AtomGroup *owner, Atom *a, Atom *b, Atom *c, 
                         Atom *d, double angle, bool constrained)
{
	_owner = owner;
	_constrained = constrained;
	_a = a;
	_b = b;
	_c = c;
	_d = d;
	_angle = angle;
	
	if (_a == nullptr || _b == nullptr || _c == nullptr || _d == nullptr)
	{
		throw(std::runtime_error("Initialising bond torsion with null values"));
	}
	
	if (owner && (!owner->hasAtom(a) || !owner->hasAtom(b) 
	    || !owner->hasAtom(c) || !owner->hasAtom(d)))
	{
		throw(std::runtime_error("Owner does not own atom "
		                         "assigned to BondTorsion"));
	}
	
	if (_owner)
	{
		_a->addBondstraint(this);
		_b->addBondstraint(this);
		_c->addBondstraint(this);
		_d->addBondstraint(this);
		_owner->addBondstraint(this);
	}
}

bool BondTorsion::operator==(const BondTorsion &other) const
{
	if (_a == other._a && _b == other._b && _c == other._c && _d == other._d)
	{
		return true;
	}

	if (_a == other._d && _b == other._c && _c == other._b && _d == other._a)
	{
		return true;
	}

	return false;
}

bool BondTorsion::coversMainChain() const
{
	return (_b->isCoreMainChain() && _c->isCoreMainChain());
}

bool BondTorsion::isConstrained() const
{
	return _constrained;
}

bool BondTorsion::isPeptideBond() const
{
	return ((_b->atomName() == "C" && _c->atomName() == "N") ||
	        (_b->atomName() == "N" && _c->atomName() == "C"));
}

const std::string BondTorsion::reverse_desc() 
{
	if (_rDesc.length()) return _rDesc;

	_rDesc = (_d->atomName() + "-" + _c->atomName() + "-" + 
	          _b->atomName() + "-" + _a->atomName());

	return _rDesc;
}

const std::string BondTorsion::desc() 
{
	if (_desc.length()) return _desc;

	_desc = (_a->atomName() + "-" + _b->atomName() + "-" + 
	         _c->atomName() + "-" + _d->atomName());
	
	return _desc;
}

const std::string BondTorsion::short_desc() 
{
	if (_sDesc.length()) return _sDesc;

	if ((_b->atomName() == "CA" && _c->atomName() == "N") ||
	    (_b->atomName() == "N" && _c->atomName() == "CA"))
	{
		_sDesc = "phi";
	}

	else if ((_b->atomName() == "CA" && _c->atomName() == "C") ||
	    (_b->atomName() == "C" && _c->atomName() == "CA"))
	{
		_sDesc = "psi";
	}

	else if ((_b->atomName() == "C" && _c->atomName() == "N") ||
	    (_b->atomName() == "N" && _c->atomName() == "C"))
	{
		_sDesc = "omega";
	}
	
	else if ((_b->atomName() == "CA" && _c->atomName() == "CB") ||
	    (_b->atomName() == "CB" && _c->atomName() == "CA"))
	{
		_sDesc = "chi1";
	}

	else if ((_b->atomName() == "CB" && std::regex_match(_c->atomName(), std::regex("CG."))) ||
	    (std::regex_match(_b->atomName(), std::regex("CG.")) && _c->atomName() == "CB"))
	{
		_sDesc = "chi2";
	}

	else if ((_b->atomName() == "CG" && std::regex_match(_c->atomName(), std::regex(".D"))) ||
	    (std::regex_match(_b->atomName(), std::regex(".D")) && _c->atomName() == "CG"))
	{
		_sDesc = "chi3";
	}

	else if ((_b->atomName() == "CD" && std::regex_match(_c->atomName(), std::regex(".E"))) ||
	    (std::regex_match(_b->atomName(), std::regex(".E")) && _c->atomName() == "CD"))
	{
		_sDesc = "chi4";
	}

	else if (_b->atomName() == "CZ" || _c->atomName() == "CZ")
	{
		_sDesc = "chi5";
	}

	return _sDesc;
}

double BondTorsion::startingAngle()
{
	if (isConstrained())
	{
		return _angle;
	}
	
	if (_working)
	{
		return _refinedAngle;
	}

	return measurement(SourceInitial);
}

double BondTorsion::measurement(BondTorsion::Source source, bool force)
{
	if (_measured && source == SourceInitial
			&& !force)
	{
		return _measuredAngle;
	}

	glm::vec3 poz[4];
	
	for (size_t i = 0; i < 4; i++)
	{
		if (source == SourceInitial)
		{
			poz[i] = atom(i)->initialPosition();
		}
		else if (source == SourceDerived)
		{
			poz[i] = atom(i)->derivedPosition();
		}
	}
	
	double putative = measure_bond_torsion(poz);
	
	if (putative != putative)
	{
		putative = 0;
	}
	
	if (source == SourceInitial)
	{
		_measured = true;
		_measuredAngle = putative;
	}
	
	return putative;
}

double BondTorsion::similarityScore(BondTorsion *const other) const
{
	glm::vec3 x = glm::vec3(1., 0., 0.);
	glm::vec3 average = bondDirection() + other->bondDirection();
	average = normalize(average);

	glm::mat3x3 twizzle = glm::inverse(mat3x3_rhbasis(average, x));
	
	if (!is_glm_vec_sane(twizzle[0]))
	{
		x = glm::vec3(0., 1., 0.);
		twizzle = glm::transpose(mat3x3_rhbasis(average, x));
	}

	double dot = 0;
	double dist = 0;

	{
		glm::vec3 mine = twizzle * bondDirection();
		glm::vec3 yours = twizzle * other->bondDirection();

		glm::vec2 my2D = glm::normalize(glm::vec2(mine.x, mine.z));
		glm::vec2 your2D = glm::normalize(glm::vec2(yours.x, yours.z));

		dot = glm::dot(my2D, your2D);
	}

	{
		glm::vec3 mine = twizzle * bondMidPoint();
		glm::vec3 yours = twizzle * other->bondMidPoint();
		mine.x = 0;
		yours.x = 0;
		

		dist = glm::length(mine - yours);
	}
	
	double dist_mult = (_maxSeparation - dist) / _maxSeparation;
	if (dist_mult < 0) dist_mult = 0;
	
	return dot * dist_mult;
}

glm::vec3 BondTorsion::bondDirection() const
{
	glm::vec3 diff = atom(2)->derivedPosition();
	diff -= atom(1)->derivedPosition();

	return diff;
}

glm::vec3 BondTorsion::bondMidPoint() const
{
	glm::vec3 sum = atom(1)->derivedPosition();
	sum += atom(2)->derivedPosition();

	sum /= 2;
	return sum;
}

const ResidueId &BondTorsion::residueId() const
{
	if (_gotId)
	{
		return _resId;
	}

	Atom *owning = owningAtom();
	return owning->residueId();
}

Atom *BondTorsion::owningAtom() const
{
	const ResidueId &bId = _b->residueId();
	const ResidueId &cId = _c->residueId();

	return (bId < cId ? _b : _c);
}

ResidueId BondTorsion::residueId() 
{
	if (_gotId)
	{
		return _resId;
	}

	ResidueId bId = _b->residueId();
	const ResidueId &cId = _c->residueId();

	_resId = (bId < cId ? bId : cId);

	return _resId;
}

bool BondTorsion::spansMultipleChains() const
{
	std::string compare = atom(0)->chain();
	
	for (size_t i = 1; i < 4; i++)
	{
		if (atom(i)->chain() != compare)
		{
			return true;
		}
	}
	
	return false;
}

bool BondTorsion::matchesAtoms(Atom *a, Atom *b, Atom *c, Atom *d)
{
	auto matches_list = [this](const std::array<Atom *, 4> &abcd)
	{
		for (int i = 0; i < 4; i++)
		{
			if (abcd[i] && abcd[i] != atom(i))
			{
				return false;
			}
		}

		return true;
	};

	return matches_list({a, b, c, d}) || matches_list({d, c, b, a});
}
