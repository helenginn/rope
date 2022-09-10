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

#include "VisualPreferences.h"
#include "Atom.h"

VisualPreferences::VisualPreferences()
{

}

bool VisualPreferences::isBallStickAtom(const Atom *a)
{
	if (_ballAndStick)
	{
		return true;
	}
	
	if (a->atomName() == "C" || a->atomName() == "O" || a->atomName() == "N"
	    || a->atomName() == "HA" || a->atomName() == "H")
	{
		// if single side chains, don't display main chain
		return false;
	}

	const ResidueId &id = a->residueId();
	for (const ResidueId &resId : _displayBallSticks)
	{
		if (resId == id)
		{
			return true;
		}
	}
	
	return false;
}

std::vector<Atom *> VisualPreferences::selectBallStickAtoms(std::vector<Atom *> 
                                                            &av)
{
	std::vector<Atom *> subset;

	for (Atom *a : av)
	{
		if (isBallStickAtom(a))
		{
			subset.push_back(a);
		}
	}

	return subset;
}
