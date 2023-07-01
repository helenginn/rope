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

float AreaMeasurer::surfaceArea()
{
	_contacts->updateSheet(_posMap);
	
	// calculate
	float area = 10;
	//fibonacci points test
	std::vector<glm::vec3> points = _lattice.getPoints();


  //loop through points and check that each point has magnitude 1:
	for (int i = 0; i < points.size(); i++)
	{
		float magnitude = sqrt(pow(points[i].x,2) + pow(points[i].y,2) + pow(points[i].z,2));
		if (fabs(magnitude - 1) > 0.0001)
		{
			std::cout << "Error: magnitude of point " << i << " is not 1, is " << magnitude << std::endl;
		}
	}
	//end fibonacci points test

	return area;
}

float AreaMeasurer::fibExposureSingleAtom(Atom *atom)
{
	_lattice.changeLatticeRadius(atom);
	std::vector<glm::vec3> points = _lattice.getPoints();
	std::vector<glm::vec3> points_in_overlap;

	//for each lattice point in fibonacci lattice
	for (const glm::vec3 &point : points)
	{
		// for every other atom in posmap
		for (auto &other_atom : _posMap)
		{
			if (other_atom.first != atom)
			{
				// check if point in overlap
				const float radius = getVdWRadius(other_atom.first);
				if (glm::length(point - other_atom.second.ave) <= radius + 1.52f) // + O-radius to account for solvent molecule size
				{
					points_in_overlap.push_back(point);
				}
			}
		}
	}
	// return percentage of points not in overlap
	return 1 - ((float) points_in_overlap.size() / points.size());
}

float AreaMeasurer::fibExposureSingleAtomZSlice(Atom *atom, float radius)
{
	_lattice.changeLatticeRadius(atom);
	std::vector<glm::vec3> points = _lattice.getPoints();
	std::vector<glm::vec3> points_in_overlap;

	std::set<Atom *> nearAtoms = _contacts->atomsNear(atom, radius);
	_contacts->calculateZSliceMap(atom, nearAtoms);
	std::map<Atom *, std::map<Atom *, std::pair<float, float> > > zSliceMap = _contacts->getZSliceMap();
	
	// for every other atom in nearAtoms
	for (const auto &other_atom : nearAtoms)
	{
		// get lower and upper z slice bounds
		const float Z_l = zSliceMap[atom][other_atom].first;
		const float Z_u = zSliceMap[atom][other_atom].second;
		//for each lattice point in fibonacci lattice
		for (const glm::vec3 &point : points)
		{
			if (other_atom != atom && point.z >= Z_l && point.z <= Z_u)
			{
				// check if point in overlap
				const float radius = getVdWRadius(other_atom);
				if (glm::length(point - other_atom->derivedPosition()) <= radius + 1.52f) // + O-radius to account for solvent molecule size
				{
					points_in_overlap.push_back(point);
				}
			}
		}
	}
	// return percentage of points not in overlap
	return 1 - ((float) points_in_overlap.size() / points.size());
}

float areaFromExposure(float exposure, Atom *atom)
{
	float radius = getVdWRadius(atom);
	float area = exposure * 4 * M_PI * pow(radius,2);
	return area;
}

float getVdWRadius(Atom *atom)
{
	std::string elementSymbol = atom->elementSymbol();
	gemmi::Element elem(elementSymbol);
	float radius = elem.vdw_r();
	return radius;
}
