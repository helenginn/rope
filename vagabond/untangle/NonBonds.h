// 
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

#ifndef __vagabond__NonBonds__
#define __vagabond__NonBonds__

#include <vector>
#include <set>
#include <map>

class Atom;

class NonBonds
{
public:
	NonBonds();

	void setAtoms(const std::vector<Atom *> &atoms);

	std::set<Atom *> acquireContacts(Atom *const &atom,
	                                 const std::vector<Atom *> &source,
	                                 float threshold);
	
	typedef std::function<void (Atom *, const std::string &, float)> 
	UpdateBadness;

	void setUpdateBadness(const UpdateBadness &update)
	{
		_update = update;
	}

	float evaluateAtom(Atom *const &atom, bool print);
private:
	std::set<Atom *> _atoms;
	std::map<Atom *, std::set<Atom *>> _contacts;
	std::map<Atom *, float> _radii;

	UpdateBadness _update{};
};

#endif
