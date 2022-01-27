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
#include "Knotter.h"
#include "BondAngle.h"
#include "BondTorsion.h"
#include "BondLength.h"
#include "Chirality.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "GeometryTable.h"

Knotter::Knotter(AtomGroup *group, GeometryTable *table)
{
	_group = group;
	_table = table;
	
	if (_group == nullptr || _table == nullptr)
	{
		throw std::runtime_error("Knotter initialised with NULL values");
	}
}

void Knotter::checkAtoms(Atom *atom, int start)
{
	AtomGroup &group = *_group;
	glm::vec3 pos = atom->derivedPosition();
	const float cutoff = 2;

	for (size_t i = start; i < group.size(); i++)
	{
		Atom *other = group[i];
		if (other == atom)
		{
			continue;
		}

		glm::vec3 diff = other->derivedPosition() - pos;
		
		for (size_t j = 0; j < 3; j++)
		{
			if (diff[j] < -cutoff || diff[j] > cutoff)
			{
				continue;
			}
		}
		
		std::string code = atom->code();
		std::string orig = atom->atomName();
		std::string compare = other->atomName();
		
		double standard = _table->length(code, orig, compare);
		
		if (standard < 0)
		{
			continue;
		}
		
		double length = glm::length(diff);
		
		if (length < standard * 1.4)
		{
			new BondLength(_group, atom, other, standard);
		}
	}

}

void Knotter::createBondAngles(Atom *atom)
{
	std::vector<Atom *> partners;
	partners.reserve(atom->bondLengthCount());

	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		partners.push_back(atom->bondLength(i)->otherAtom(atom));
	}

	for (size_t i = 0; i < partners.size() - 1; i++)
	{
		Atom *start = partners[i];
		for (size_t j = i + 1; j < partners.size(); j++)
		{
			Atom *end = partners[j];

			std::string sName = start->atomName();
			std::string aName = atom->atomName();
			std::string eName = end->atomName();
			std::string aCode = atom->code();

			double standard = _table->angle(aCode, sName, aName, eName);

			if (standard < 0)
			{
				std::string warning = "Warning: missing angle for " +
				 sName + ", " + aName + ", " + eName + 
				" for central atom code " + aCode;

				_warnings.push_back(warning);
				continue;
			}

			BondAngle *angle = new BondAngle(_group, start, atom, end, standard);
		}
	}
}

void Knotter::createBondTorsion(BondAngle *first, BondAngle *second)
{
	Atom *a, *b, *c, *d;
	first->getSequentialAtoms(second, &a, &b, &c, &d);
	
	std::string aName = a->atomName();
	std::string bName = b->atomName();
	std::string cName = c->atomName();
	std::string dName = d->atomName();
	std::string code = b->code();

	double standard = _table->torsion(code, aName, bName, cName, dName);
	double dev = _table->torsion_stdev(code, aName, bName, cName, dName);
	
	BondTorsion *torsion = new BondTorsion(_group, a, b, c, d, standard);

	/* if dev is less than 0.5 degrees, treat as constraint */
	if (standard >= 0 && dev < 0.5) 
	{
		torsion->setConstrained(true);
	}

}

void Knotter::findBondTorsions()
{
	AtomGroup &group = *_group;

	for (int i = 0; i < (int)group.bondAngleCount() - 1; i++)
	{
		BondAngle *first = group.bondAngle(i);
		
		for (int j = i + 1; j < (int)group.bondAngleCount(); j++)
		{
			BondAngle *second = group.bondAngle(j);
			
			bool overlaps = first->formsTorsionWith(second);
			
			if (overlaps)
			{
				createBondTorsion(first, second);
			}
		}
	}
}

void Knotter::findBondAngles()
{
	AtomGroup &group = *_group;
	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *atom = group[i];
		
		if (atom->bondLengthCount() > 1)
		{
			createBondAngles(atom);
		}
	}
}

void Knotter::findBondLengths()
{
	AtomGroup &group = *_group;
	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *atom = group[i];
		checkAtoms(atom, i + 1);
	}
}

void Knotter::checkAtomChirality(Atom *atom)
{
	Atom *atoms[4] = {nullptr, nullptr, nullptr, nullptr};
	std::string code = atom->code();
	std::string centre = atom->atomName();

	for (size_t i = 0; i < 4; i++)
	{
		int n = 0;
		for (size_t j = 0; j < atom->bondLengthCount(); j++)
		{
			if (j == i)
			{
				continue;
			}

			atoms[n] = atom->connectedAtom(j);
			n++;
		}

		bool skip = false;
		for (size_t j = 0; j < 3; j++)
		{
			if (atoms[j] == nullptr)
			{
				skip = true;
			}
		}
		
		if (skip)
		{
			continue;
		}

		std::string pName = atoms[0]->atomName();
		std::string qName = atoms[1]->atomName();
		std::string rName = atoms[2]->atomName();

		int sign = _table->chirality(code, atom->atomName(), pName, qName, rName);

		if (sign != 0)
		{
			new Chirality(_group, atom, atoms[0], atoms[1], atoms[2], sign);
			break;
		}
	}
}

void Knotter::findChiralCentres()
{
	AtomGroup &group = *_group;
	for (size_t i = 0; i < group.size(); i++)
	{
		if (group[i]->bondLengthCount() >= 3)
		{
			checkAtomChirality(group[i]);
		}
	}

}

void Knotter::knot()
{
	findBondLengths();
	findBondAngles();
	findBondTorsions();
	findChiralCentres();
}

