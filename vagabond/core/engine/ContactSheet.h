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

#ifndef __vagabond__ContactSheet__
#define __vagabond__ContactSheet__

#include <set>
#include "Atom.h"
#include "../utils/glm_import.h"

/** \class ContactSheet
 * \brief class to pre-calculate and then query inter-atomic contacts
 * within a certain radius. */

class ContactSheet
{
public:
	/** initialise contact sheet */
	ContactSheet();

	/** when new atom positions have been obtained, update an existing contact
	 * sheet for these positions before sending any queries. This can take
	 * advantage of any partially-sorted existing sheet calculations as updated
	 * atom positions will be closely related. */
	void updateSheet(AtomPosMap &newPositions);

	/** calculate zSliceMap for relevant atom neighbours*/
	void calculateZSliceMap(Atom * centre, std::set<Atom *> nearAtoms);
	
	std::map<Atom *, std::map<Atom *, std::pair<float, float> > > getZSliceMap() const
	{
		return _zSliceMap;
	}

	/** return set of near atoms within the specified radius*/
	std::set<Atom *> atomsNear(Atom *centre, float radius);
private:
	AtomPosMap _posMap;
	std::map<Atom *, std::map<Atom *, std::pair<float, float> > > _zSliceMap;
};

#endif
