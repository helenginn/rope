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

#ifndef __vagabond__ShortRoute__
#define __vagabond__ShortRoute__

#include "PlausibleRoute.h"

struct AtomGraph;
class Atom;

class ShortRoute : public PlausibleRoute
{
public:
	ShortRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims);

	virtual void setup();
	bool prepareSegment(Atom *atom);
	
	void setAtom(Atom *atom, int count)
	{
		_atom = atom;
		_count = count;
	}
	
	const BondSequence *sequence() const
	{
		return _sequence;
	}
private:
	Atom *_atom = nullptr;
	const BondSequence *_sequence = nullptr;
	int _count = 10;

};

#endif
