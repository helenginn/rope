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

#ifndef __vagabond__Knotter__
#define __vagabond__Knotter__

class Atom;
class AtomGroup;
class GeometryTable;
class BondAngle;

#include <vector>
#include <string>

class Knotter
{
public:
	Knotter(AtomGroup *group, GeometryTable *table);
	~Knotter();

	void knot();
	
	void setDoLengths(bool doit)
	{
		_doLengths = doit;
	}
	
	void setDoAngles(bool doit)
	{
		_doAngles = doit;
	}
	
	void setDoTorsions(bool doit)
	{
		_doTorsions = doit;
	}
	
	std::string warning(int i)
	{
		return _warnings[i];
	}
	
	size_t warningCount()
	{
		return _warnings.size();
	}
private:
	void findBondLengths();
	void findBondAngles();
	void findBondTorsions();
	void findChiralCentres();
	void createBondAngles(Atom *atom);
	void createBondTorsion(BondAngle *first, BondAngle *second);
	void checkAtomChirality(Atom *atom, bool use_dictionary);
	void checkAtoms(Atom *atom, int start);

	AtomGroup *_group;
	GeometryTable *_table;
	std::vector<std::string> _warnings;

	bool _doLengths = true;
	bool _doAngles = true;
	bool _doTorsions = true;
};

#endif
