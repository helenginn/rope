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

#include "AreaMeasurer.h"
#include "ContactSheet.h"
#include "vagabond/core/Fibonacci.h"
#include <string>
#include <iostream>
#include <gemmi/elem.hpp>


AreaMeasurer::AreaMeasurer(SurfaceAreaHandler *handler, int n_points)
{
	_handler = handler;
	_contacts = new ContactSheet();
	_lattice = Fibonacci();
  _lattice.generateLattice(n_points, 1.0);
}

AreaMeasurer::~AreaMeasurer()
{
	delete _contacts;
}

float AreaMeasurer::surfaceArea(const AtomPosMap &posMap)
{
	TimerSurfaceArea& timer = TimerSurfaceArea::getInstance();
	bool timing = timer.timing;
	
	if (timing)
	{	 timer.start();}

	// _contacts->updateSheet(_posMap); // not necessary
	_lattice.resetLatticeRadius();
	// std::cout << _posMap.size() << std::endl;

	AreaMeasurer::setProbeRadius(1.35);
	
	// calculate
	float area = 0;
	//fibonacci points test
	std::vector<glm::vec3> points = _lattice.getPoints();


  // //loop through points and check that each point has magnitude 1:
	// for (int i = 0; i < points.size(); i++)
	// {
	// 	float magnitude = sqrt(pow(points[i].x,2) + pow(points[i].y,2) + pow(points[i].z,2));
	// 	if (fabs(magnitude - 1) > 0.0001)
	// 	{
	// 		std::cout << "Error: magnitude of point " << i << " is not 1, is " << magnitude << std::endl;
	// 	}
	// }
	// //end fibonacci points test

  for (std::pair<Atom *const, WithPos> atom : posMap)
	{
		const float radius = getVdWRadius(atom.first);
    const float &exposure = AreaMeasurer::fibExposureSingleAtom(posMap, atom.first, radius); // pass in posmap to this function
		const float &area_atom = areaFromExposure(exposure, radius, _probeRadius);
		area += area_atom;
	}

  if (timing)
	{	timer.end();}

	return area;
}

float AreaMeasurer::fibExposureSingleAtom(const AtomPosMap &posMap, Atom *atom, const float radius) // also needs local copy of posmap
{
	_lattice.changeLatticeRadius(radius, _probeRadius);
	std::vector<glm::vec3> points = _lattice.getPoints();
	int points_in_overlap = 0;
	const glm::vec3 pos = atom->derivedPosition();

	//for each lattice point in fibonacci lattice
	for (const glm::vec3 &point : points)
	{
		// for every other atom in posmap
		for (auto &other_atom : _posMap)
		{
			if (other_atom.first != atom)
			{
				const float radius = getVdWRadius(other_atom.first);
				// check if point in overlap
				// if (glm::length(point + pos - other_atom.second.ave) <= radius + _probeRadius + 1e-6f) // + probe radius to account for solvent molecule size
				if (sqlength(point + pos - other_atom.first->derivedPosition()) <= (radius + _probeRadius + 1e-6f) * (radius + _probeRadius + 1e-6f)) // + probe radius to account for solvent molecule size
				{
					points_in_overlap++;
					break;
				}
			}
			// std::cout << "Other atom " << other_atom.first->elementSymbol() << "points in overlap: " << points_in_overlap.size() << std::endl;
		}
	}
	// std::cout <<  "points in overlap: " << points_in_overlap.size() << std::endl;
	// std::cout << "percentage of points not in overlap: " << 1 - ((float) points_in_overlap.size() / points.size()) << std::endl;
	// return percentage of points not in overlap
	return 1 - ((float) points_in_overlap / points.size());
}

float AreaMeasurer::fibExposureSingleAtomZSlice(const AtomPosMap &posMap, Atom *atom, const float radius)
{
	_lattice.changeLatticeRadius(radius, _probeRadius);
	std::vector<glm::vec3> points = _lattice.getPoints();
	int points_in_overlap = 0;
	const glm::vec3 pos = atom->derivedPosition();

	std::set<Atom *> nearAtoms = _contacts->atomsNear(posMap, atom, radius); //todo
	_contacts->calculateZSliceMap(atom, nearAtoms);
	std::map<Atom *, std::map<Atom *, std::pair<float, float> > > zSliceMap = _contacts->getZSliceMap();
	
	// for every other atom in nearAtoms
	for (const auto &other_atom : nearAtoms)
	{
		// get lower and upper z slice bounds
		const float Z_l = zSliceMap[atom][other_atom].first; //make 1 reference for both calls
		const float Z_u = zSliceMap[atom][other_atom].second;
		//for each lattice point in fibonacci lattice
		for (const glm::vec3 &point : points)
		{
			if (other_atom != atom && point.z >= Z_l && point.z <= Z_u)
			{
				// check if point in overlap
				if (sqlength(point + pos - other_atom->derivedPosition()) <= (radius + _probeRadius + 1e-6f) * (radius + _probeRadius + 1e-6f)) // + probe radius to account for solvent molecule size ;replace with posmap value (derivedpos)
				{
					points_in_overlap++;
					break;
				}
			}
		}
	}
	// return percentage of points not in overlap
	return 1 - ((float) points_in_overlap / points.size());
}

float areaFromExposure(float exposure, float radius, double probeRadius)
{
	const float area = exposure * 4 * M_PI * (radius + probeRadius) * (radius + probeRadius);
	return area;
}

float getVdWRadius(Atom *atom)
{
	std::string elementSymbol = atom->elementSymbol();
	gemmi::Element elem(elementSymbol);
	const float radius = elem.vdw_r();
	return radius;
}
