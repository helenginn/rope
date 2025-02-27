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

#include "NonBonds.h"
#include <vagabond/core/Atom.h>
#include <vagabond/core/BondAngle.h>
#include <vagabond/core/BondTorsion.h>
#include <gemmi/elem.hpp>

NonBonds::NonBonds()
{

}

std::set<Atom *> NonBonds::acquireContacts(Atom *const &atom,
                                           const std::vector<Atom *> &source,
                                           float threshold)
{
	std::set<Atom *> contacts;
	Atom::ConformerInfo pInfo = atom->conformerPositions();

	for (Atom *possible : source)
	{
		if (possible == atom)
		{
			continue;
		}
		
		bool too_close = false;
		for (size_t k = 0; k < atom->bondAngleCount(); k++)
		{
			if (atom->bondAngle(k)->hasAtom(possible))
			{
				too_close = true;
			}
		}

		for (size_t k = 0; k < atom->bondTorsionCount(); k++)
		{
			if (atom->bondTorsion(k)->isConstrained() && 
			    atom->bondTorsion(k)->hasAtom(possible))
			{
				too_close = true;
			}
		}

		if (too_close) continue;

		Atom::ConformerInfo qInfo = possible->conformerPositions();
		
		bool found = false;
		for (auto it = pInfo.begin(); it != pInfo.end() && !found; it++)
		{
			glm::vec3 p = it->second.pos.ave;

			for (auto jt = qInfo.begin(); jt != qInfo.end() && !found; jt++)
			{
				glm::vec3 q = jt->second.pos.ave;

				if (fabs(p.x - q.x) > threshold) { continue; }
				if (fabs(p.y - q.y) > threshold) { continue; }
				if (fabs(p.z - q.z) > threshold) { continue; }
				
				if (glm::length(p - q) > threshold)
				{
					continue;
				}

				contacts.insert(possible);
				found = true;
				break;
			}
		}
	}
	
	return contacts;
}

void NonBonds::setAtoms(const std::vector<Atom *> &atoms)
{
	for (Atom *const &atom : atoms)
	{
		_atoms.insert(atom);
		std::set<Atom *> contacts = acquireContacts(atom, atoms, 5.f);
		_contacts[atom] = contacts;

		std::string mySymbol = atom->elementSymbol();
		gemmi::Element ele = gemmi::Element(mySymbol);
		float myVdwRadius = ele.vdw_r();
		_radii[atom] = myVdwRadius;
		
		_byResidue[{atom->chain(), atom->residueId()}].push_back(atom);
		
		for (size_t k = 0; k < atom->bondTorsionCount(); k++)
		{
			Atom *a = atom->bondTorsion(k)->atom(1);
			Atom *b = atom->bondTorsion(k)->atom(2);
			if (a->elementSymbol() == "S" && b->elementSymbol() == "S")
			{
				_exclude.insert({{a->chain(), a->residueId()},
			                     {b->chain(), b->residueId()}});
			}
		}
	}
}

template <class It>
struct increment_atom
{
	increment_atom(const It &begin, const It &end) :
	_begin(begin), _end(end), _increment(begin)
	{}

	Atom *operator()()
	{
		if (_increment == _end)
		{
			_increment = _begin;
			return nullptr;
		}

		It copy = _increment;
		_increment++;
		return *copy;
	}

	It _begin;
	It _end;
	It _increment;
};

float NonBonds::evaluateResiduePair(const ChainResi &a, 
                                    const ChainResi &b)
{
	if (_exclude.count({a, b}) || _exclude.count({b, a}))
	{
		return 0;
	}

	increment_atom<std::vector<Atom *>::iterator> a_atoms = 
	increment_atom<std::vector<Atom *>::iterator>(_byResidue[a].begin(),
	                                              _byResidue[a].end());

	increment_atom<std::vector<Atom *>::iterator> b_atoms = 
	increment_atom<std::vector<Atom *>::iterator>(_byResidue[b].begin(),
	                                              _byResidue[b].end());

	Atom *atom = nullptr;
	float sum = 0;
	while (true)
	{
		atom = a_atoms();
		if (!atom) break;
		
		sum += evaluateAtom(atom, false, false, b_atoms);
	}
	
	return sum;
}

float NonBonds::evaluateAtom(Atom *const &atom, bool print)
{
	increment_atom<std::set<Atom *>::iterator> inc = 
	increment_atom<std::set<Atom *>::iterator>(_contacts[atom].begin(),
	                                           _contacts[atom].end());
	
	return evaluateAtom(atom, print, true, inc);
}

float NonBonds::evaluateAtom(Atom *const &atom, bool print, bool accumulate,
                             const std::function<Atom *()> &acquireAtom)
{
	float myVdwRadius = _radii[atom];
	float total = 0;
	
	if (print) 
	{
		std::cout << "Non-bonding for " << atom->desc() << std::endl;
	}

	for (auto it = atom->conformerPositions().begin();
	     it != atom->conformerPositions().end(); it++)
	{
		float sum = 0;
		std::string conf = it->first;
		if (print)
		{
			std::cout << "Conformer " << conf << ":" << std::endl;
		}
		glm::vec3 pos = it->second.pos.ave;
		int count = 0;
		
		Atom *contact = nullptr;
		while (true)
		{
			contact = acquireAtom();
			if (contact == nullptr)
			{
				break;
			}
			
			if (atom == contact || 
			    contact->conformerPositions().count(conf) == 0)
			{
				continue;
			}

			glm::vec3 compare = contact->conformerPositions()[conf].pos.ave;

			float otherVdwRadius = _radii[contact];
			float target = otherVdwRadius + myVdwRadius;
			target *= 0.7;
			
			glm::vec3 diff = compare - pos;
			float actual = glm::length(diff);
			if (actual < target)
			{
				if (print)
				{
					std::cout << "\t" << std::setw(8) << atom->desc() << 
					" / " << std::setw(8) << contact->desc();
				}

				float ratio = target / actual;
				long double to12 = ratio * ratio * ratio;
				to12 *= to12;
				
				if (!std::isfinite(to12))
				{
					std::cout << "problem" << std::endl;
				}

				if (print)
				{
					std::cout << " : " << std::setw(8) << to12 << std::endl;
				}

				sum += to12;
				count++;
			}
		}
		
		if (count == 0) sum = 0;
		total += sum;

		if (accumulate)
		{
			_update(atom, conf, sum);
		}
	}

	if (print)
	{
		std::cout << std::endl;
	}
	
	if (!std::isfinite(total))
	{
//		std::cout << "Warning! Total is not finite: " << atom->desc() << std::endl;
	}

	return total;
}

std::set<ChainResi> NonBonds::residueIds()
{
	std::set<ChainResi> ids;
	for (auto it = _byResidue.begin(); it != _byResidue.end(); it++)
	{
		ids.insert(it->first);
	}
	return ids;
}
