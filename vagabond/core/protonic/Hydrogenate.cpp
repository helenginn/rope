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

using namespace hnet;

Hydrogenate::Hydrogenate(::Atom *atom, AtomGroup *destination)
{
	_atom = atom;
	_destination = destination;
}

void Hydrogenate::operator()()
{
	if (_atom->elementSymbol() == "H")
	{
		return;
	}

	int coordNum = 0;

	OpSet<::Atom *> to_align = inactiveHydrogens(_atom, coordNum);

	if (coordNum == 0 || to_align.size() == 0 ||
	    (coordNum > 2 && to_align.size() == 1))
	{
		return;
	}
	
	glm::vec3 centre = _atom->initialPosition();

	std::vector<glm::vec3> some;
	for (::Atom *const &atom : to_align)
	{
		some.push_back(atom->initialPosition());
	}
	
	std::vector<glm::vec3> all = align(coordNum, centre, some);
	
	if (coordNum > 10)
	{
		coordNum -= 9;
	}
	
	std::vector<std::string> names = inactiveHydrogenNames(_atom);
	int n = 0;
	
	if (names.size() != (coordNum - some.size()))
	{
		std::cout << "Warning: name numbers don't match!" << std::endl;
	}

	for (size_t i = some.size(); i < coordNum; i++)
	{
		::Atom *hAtom = new ::Atom();
		hAtom->setResidueId(_atom->residueId());
		hAtom->setInitialPosition(all[i]);
		hAtom->setCode(_atom->code());
		hAtom->setAtomName(names[n]); n++;
		hAtom->setElementSymbol("H");
		std::cout << "\tnew H for " << _atom->desc() << " at " 
		<< glm::to_string(all[i]) << std::endl;
		*_destination += hAtom;
	}
}
