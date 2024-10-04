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
#include "HyperValue.h"
#include "BondTorsion.h"
#include "BondLength.h"
#include "Chirality.h"
#include "AtomGroup.h"
#include "HelpKnot.h"
#include "Atom.h"
#include "programs/RingProgrammer.h"
#include "GeometryTable.h"

Knotter::Knotter(AtomGroup *group, GeometryTable *table) 
: _programmers(*RingProgrammer::allProgrammers())
{
	_group = group;
	_table = table;
	
	if (_group == nullptr || _table == nullptr)
	{
		throw std::runtime_error("Knotter initialised with nullptr values");
	}
}

Knotter::~Knotter()
{

}

void Knotter::checkAtoms(HelpKnot &knot, Atom *atom, int start)
{
	glm::vec3 pos = atom->derivedPosition();

	const float cutoff = 3;
	std::vector<Atom *> &atoms = knot.atoms(pos);

	for (Atom *other : atoms)
	{
		if (other == atom)
		{
			continue;
		}

		glm::vec3 diff = other->derivedPosition() - pos;
		
		bool skip = false;
		for (size_t j = 0; j < 3; j++)
		{
			if (fabs(diff[j]) > cutoff)
			{
				skip = true;
			}
		}
		
		if (skip)
		{
			continue;
		}

		std::string code = atom->code();
		std::string other_code = other->code();

		std::string orig = atom->atomName();
		std::string compare = other->atomName();
		
		double standard = -1;
		
		if ((atom->code() == "HOH" && other->code() != "HOH") ||
		    (atom->code() != "HOH" && other->code() == "HOH"))
		{
			continue;
		}

		if (atom->residueId() == other->residueId()
		    && atom->chain() == other->chain())
		{
			standard = _table->length(code, orig, compare, false);
		}
		else
		{
			standard = _table->length(code, orig, compare, true);

			if (standard < 0)
			{
				standard = _table->length(other->code(), compare, orig, true);
			}
		}

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

			double standard = -1;
			double stdev = -1;
			
			if (start->residueId() == end->residueId() 
			    && start->chain() == end->chain())
			{
				standard = _table->angle(aCode, sName, aName, eName);
				stdev = _table->angle_stdev(aCode, sName, aName, eName);
			}
			else
			{
				standard = _table->angle(aCode, sName, aName, eName, true);
				stdev = _table->angle_stdev(aCode, sName, aName, eName, true);
			}

			if (standard < 0)
			{
				std::string warning = "Warning: missing angle for " +
				 sName + ", " + aName + ", " + eName + 
				" for central atom code " + aCode;

				_warnings.push_back(warning);
				continue;
			}

			new BondAngle(_group, start, atom, end, standard, stdev);
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

	/* if dev is less than 0.5 degrees, treat as constraint */
	bool constrained = (standard >= 0 && dev < 0.5);
	
	BondTorsion *torsion = new BondTorsion(_group, a, b, c, d, 
	                                       standard, constrained);

}

void Knotter::createHyperValues(Atom *atom, RingProgrammer *programmer)
{
	for (size_t i = 0; i < programmer->paramSpecCount(); i++)
	{
		float def = 0;
		std::string name = programmer->paramSpec(i, &def);
		HyperValue *hv = new HyperValue(_group, atom, name, def);
		
		programmer->updateValue(_group, hv, i);
	}
}

void Knotter::findBondTorsions()
{
	AtomGroup &group = *_group;

	for (int i = 0; i < (int)group.bondAngleCount() - 1; i++)
	{
		BondAngle *first = group.bondAngle(i);
		Atom *centre = first->atom(1);
		
		for (int j = 0; j < (int)centre->bondAngleCount(); j++)
		{
			BondAngle *second = centre->bondAngle(j);
			
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

void Knotter::findHyperValues()
{
	AtomGroup &group = *_group;
	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *atom = group[i];
		
		for (RingProgrammer *programmer : _programmers)
		{
			const std::string &pin = programmer->pinnedAtom();
			const std::string &code = programmer->code();

			if (atom->atomName() == pin && atom->code() == code)
			{
				createHyperValues(atom, programmer);
			}
		}
	}
}

void Knotter::findBondLengths()
{
	AtomGroup &group = *_group;
	HelpKnot knot(&group, 20);

	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *atom = group[i];
		
		std::string code = atom->code() + ":" + atom->atomName();
		BackboneType type = _table->backboneType(code);
		atom->setOverrideType(type);

		checkAtoms(knot, atom, i + 1);
	}
}

void Knotter::checkAtomChirality(Atom *atom, bool use_dictionary)
{
	Atom *atoms[4] = {nullptr, nullptr, nullptr, nullptr};
	std::string code = atom->code();
	std::string centre = atom->atomName();

	for (size_t i = 0; i < 4; i++)
	{
		int n = 0;
		for (size_t j = 0; j < atom->bondLengthCount() && j < 4; j++)
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
		
		if (_group->findChirality(atom, atoms[0], atoms[1], atoms[2]))
		{
			continue;
		}

		int sign = _table->chirality(code, atom->atomName(), pName, qName, rName);
		
		if ((use_dictionary && sign != 0) || !use_dictionary)
		{
			new Chirality(_group, atom, atoms[0], atoms[1], atoms[2], sign);
		}
	}
}

void Knotter::findChiralCentres()
{
	AtomGroup &group = *_group;
	/* first, only assign chirality from the geometry tables */
	for (size_t i = 0; i < group.size(); i++)
	{
		if (group[i]->bondLengthCount() >= 3)
		{
			checkAtomChirality(group[i], true);
		}
	}

	/* fill in gaps with those which will have to be measured from
	 * initial positions, particularly an issue for hydrogen atoms */
	for (size_t i = 0; i < group.size(); i++)
	{
		if (group[i]->bondLengthCount() < 3)
		{
			continue;
		}

		// HARDCODE
		if (group[i]->atomName() == "C")
		{
			continue;
		}

		checkAtomChirality(group[i], false);
	}
}

void Knotter::knot()
{
	if (_doLengths)
	{
		findBondLengths();
	}

	if (_doAngles)
	{
		findBondAngles();

		if (_doTorsions)
		{
			findBondTorsions();
			findChiralCentres();
			findHyperValues();
		}
	}
}

