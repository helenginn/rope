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

#ifndef __vagabond__RouteValidator__
#define __vagabond__RouteValidator__

#include <map>
#include <string>

class PlausibleRoute;
class Instance;
class Atom;

class RouteValidator
{
public:
	RouteValidator(PlausibleRoute &route);
	
	/** does the start molecule produce something within 0.5 Angstroms of the
	 * end molecule when we use the end molecule's torsion angles? */
	std::string validate();
	
	const float &rmsd()
	{
		return _rmsd;
	}
	
	bool valid() const
	{
		return (_rmsd < 0.5);
	}
	
	/** returns average curvature per atom for a linear torsion angle trajectory */
	float linearityRatio();

	/** how many gaps are there in the definition of the end instance 
	 * 	molecule? */
	int endInstanceGaps();
private:
	float validate(Instance *start, Instance *end);

	PlausibleRoute &_route;
	std::map<Atom *, float> _distances;

	float _tolerance = 0.02;
	float _rmsd = -1;
	int _steps = 32;
};

#endif
