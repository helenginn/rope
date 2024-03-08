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
#include "Chirality.h"
#include "AtomGroup.h"
#include "Atom.h"

Chirality::Chirality(AtomGroup *owner, Atom *cen, Atom *a, Atom *b, Atom *c,
                     int sign)
{
	_owner = owner;
	_a = a;
	_b = b;
	_c = c;
	_cen = cen;
	_sign = sign;

	if (_cen == nullptr || _a == nullptr || _b == nullptr || _c == nullptr)
	{
		throw(std::runtime_error("Initialising chirality with null values"));
	}
	
	if (owner && (!owner->hasAtom(a) || !owner->hasAtom(b) 
	              || !owner->hasAtom(c) || !owner->hasAtom(cen)))
	{
		throw(std::runtime_error("Owner does not own atom assigned to Chirality"));
	}
	
	if (owner && _sign == 0)
	{
		glm::vec3 v1 = _a->initialPosition() - _cen->initialPosition();
		glm::vec3 v2 = _b->initialPosition() - _cen->initialPosition();
		glm::vec3 v3 = _c->initialPosition() - _cen->initialPosition();

		glm::vec3 cr = glm::cross(v2, v3);
		double dot = glm::dot(cr, v1);
		_sign = (dot > 0 ? 1 : -1);
	}
	
	if (_owner)
	{
		_cen->addBondstraint(this);
		_owner->addBondstraint(this);
	}
}

Chirality::~Chirality()
{

}

const bool Chirality::operator==(const Chirality &other) const
{
	if (_cen != other._cen)
	{
		return false;
	}
	
	bool found[3] = {false, false, false};
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (atom(j) == other.atom(i))
			{
				found[j] = true;
			}
		}
	}
	
	for (size_t i = 0; i < 3; i++)
	{
		if (!found[i])
		{
			return false;
		}
	}

	return true;

}

const std::string Chirality::desc()
{
	return _cen->atomName() + ":" + _a->atomName() + "-" + _b->atomName()
	+ "-" + _c->atomName();
}

int Chirality::get_sign(Atom **a, Atom **b, Atom **c, Atom **d)
{
	Atom **chosen[4] = {a, b, c, d};
	int num_found = 0;

	for (size_t i = 0; i < 4; i++)
	{
		bool found = false;
		for (size_t j = 0; j < 3; j++)
		{
			if ((*chosen[i]) == atom(j))
			{
				num_found++;
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			*chosen[i] = nullptr;
		}
	}
	
	if (num_found != 3)
	{
		return 0;
	}
	
	Atom *as[3] = {nullptr, nullptr, nullptr};
	int n = 0;
	for (size_t i = 0; i < 4; i++)
	{
		if (*chosen[i] == nullptr)
		{
			continue;
		}
		
		as[n] = *chosen[i];
		n++;
	}
	
	int mult = 0;
	
	if ((as[0] == _a && as[1] == _b && as[2] == _c) ||
	    (as[2] == _a && as[0] == _b && as[1] == _c) ||
	    (as[1] == _a && as[2] == _b && as[0] == _c))
	{
		mult = 1;
	}
	else if ((as[1] == _a && as[0] == _b && as[2] == _c) ||
	    (as[0] == _a && as[2] == _b && as[1] == _c) ||
	    (as[2] == _a && as[1] == _b && as[0] == _c))
	{
		mult = -1;
	}

	return mult * _sign;
}
