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

#include "AtomGroup.h"
#include "alignment.h"
#include "Hydrogenate.h"
#include "BondLength.h"

using namespace hnet;

Hydrogenate::Hydrogenate(::Atom *atom, AtomGroup *destination)
{
	_atom = atom;
	_destination = destination;
}

void Hydrogenate::operator()()
{
	if (!_atom || _atom->elementSymbol() == "H")
	{
		return;
	}
	
	std::vector<::Atom *> to_purge;
	for (int i = 0; i < _atom->bondLengthCount(); i++)
	{
		::Atom *other = _atom->connectedAtom(i);
		if (other->elementSymbol() == "H")
		{
			to_purge.push_back(other);
		}
	}

	for (::Atom *to_go : to_purge)
	{
		_atom->purgeConnectionsToAtom(to_go);
	}

	int coordNum = 0;
	OpSet<::Atom *> to_align = inactiveHydrogens(_atom, coordNum);

	if (coordNum == 0 || to_align.size() == 0 ||
	    (coordNum > 2 && to_align.size() == 1))
	{
		return;
	}

	auto acquire_positions_for_conf = [coordNum, to_align, this](char conf)
	{
		AtomConf ac = {_atom, conf};
		glm::vec3 centre = ac.soft_position();

		std::vector<glm::vec3> some;
		for (::Atom *const &atom : to_align)
		{
			AtomConf tmp = {atom, conf};
			some.push_back(tmp.soft_position());
		}

		std::vector<glm::vec3> all = align(coordNum, centre, some);
		return all;
	};

	
	std::vector<std::string> names = inactiveHydrogenNames(_atom);
	int n = 0;
	
	if (coordNum > 10)
	{
		coordNum -= 10;
	}

	if (names.size() != (coordNum - to_align.size()))
	{
		std::cout << "Warning: name numbers don't match!" << std::endl;
	}

	for (size_t i = to_align.size(); i < coordNum; i++)
	{
		::Atom *hAtom = new ::Atom();
		hAtom->setResidueId(_atom->residueId());

		for (const std::string &conformer : _atom->conformerList())
		{
			char conf = char_from_conf(conformer);
			std::vector<glm::vec3> all = acquire_positions_for_conf(conf);
			hAtom->conformerPositions()[conformer].pos.ave = all[i];
			hAtom->conformerPositions()[conformer].occ = 
			_atom->conformerPositions()[conformer].occ;

			std::cout << "\tnew H for " << _atom->desc() << "," << conformer
			<< " at " << glm::to_string(all[i]) << std::endl;
		}

		hAtom->setChain(_atom->chain());
		hAtom->setCode(_atom->code());
		hAtom->setAtomName(names[n]); n++;
		hAtom->setElementSymbol("H");
		std::cout << "Created new atom! " << hAtom->desc() << std::endl;

		*_destination += hAtom;
		new BondLength(nullptr, _atom, hAtom, 0.98, 0.01);
	}
}
