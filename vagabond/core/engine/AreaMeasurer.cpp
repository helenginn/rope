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

	float area = 0;

	_contacts->updateSheet(posMap);

  for (std::pair<Atom *const, WithPos> atom : posMap)
	{
		const float radius = getVdWRadius(atom.first);
		const std::set<Atom *> atomsNear = _contacts->atomsNear(
				posMap,
				atom.first,
				radius +_maxVdWRadius + 2*_probeRadius
		);
    const float &exposure = AreaMeasurer::fibExposureSingleAtom(atomsNear,
																																atom.first,
																																radius);
		const float &area_atom = areaFromExposure(exposure, radius, _probeRadius);
		area += area_atom;
	}

  if (timing)
	{	timer.end();}

	return area;
}

float AreaMeasurer::fibExposureSingleAtom(const std::set<Atom *> &atomsNear,
																					Atom *atom, const float radius)
{
	_lattice.changeLatticeRadius(radius, _probeRadius);
	std::vector<glm::vec3> points = _lattice.getPoints();
	int points_in_overlap = 0;
	const glm::vec3 pos = atom->derivedPosition();

	//for each lattice point in fibonacci lattice
	for (const glm::vec3 &point : points)
	{
		// for each other atom in posmap
		for (const auto &other_atom : atomsNear)
		{
			if (sqlength((pos-other_atom->derivedPosition())) >
					(radius+2*_probeRadius+_maxVdWRadius) *
					(radius+2*_probeRadius+_maxVdWRadius)) // if atom is too far away, skip
			{
				continue;
			}
			const float radius = getVdWRadius(other_atom);
			// check if point in overlap
			if (sqlength(point + pos - other_atom->derivedPosition())
					<= (radius + _probeRadius) 
					* (radius + _probeRadius)
					+ 1e-6f) // small epsilon so that points on the edge are counted
			{
				points_in_overlap++;
				break;
			}
		}
	}
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
				const float radius = getVdWRadius(other_atom);
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
	const float area =
		exposure * 4 * M_PI * (radius + probeRadius) * (radius + probeRadius);
	return area;
}

float getVdWRadius(Atom *atom)
{
	std::string elementSymbol = atom->elementSymbol();
	gemmi::Element elem(elementSymbol);
	const float radius = elem.vdw_r();
	return radius;
}
