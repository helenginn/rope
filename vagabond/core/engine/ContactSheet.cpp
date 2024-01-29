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

#include "ContactSheet.h"
#include "AreaMeasurer.h"
#include <cmath>

ContactSheet::ContactSheet()
{
}

void ContactSheet::updateSheet(const AtomPosMap &newPositions)
{
	// update any existing scratch calculations...
	//loop through newPositions and update _posMap:
	_posMap = newPositions;
}

// Algorithm from:
// ICM - a new method for protein modelling and design: applications to docking
// and structure prediction from the distorted native conformation.
// Abagyan et al, Journal of Computational Chemistry 15:5 488-506 (1994).

void ContactSheet::calculateZSliceMap(Atom * centre, std::set<Atom *> nearAtoms)
{
	for (Atom *const &atom : nearAtoms)
	{
		const glm::vec3 &a = centre->derivedPosition();
		const glm::vec3 &b = atom->derivedPosition();
		const float &R_A = getVdWRadius(centre);
		const float &R_B = getVdWRadius(atom);
		const float &d_AB = glm::length(centre->derivedPosition() - atom->derivedPosition());
		const float &Z_b = b.z - a.z;
		const float &r_b_sq = (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
		const float &Z_c = 0.5 * Z_b * (((R_A*R_A - R_B*R_B) / (d_AB*d_AB)) - 1);
		const float &delta = sqrt(r_b_sq * (((R_B*R_B)/(d_AB*d_AB)) - ((Z_c*Z_c)/(Z_b*Z_b))));
		const float &Z_l = (r_b_sq*(Z_c/Z_b) + delta*Z_b) > 0 ? Z_c - delta : - R_A;
		const float &Z_u = (r_b_sq*(Z_c/Z_b) - delta*Z_b) <= 0 ? R_A : Z_c + delta;

		_zSliceMap[centre][atom] = {Z_l, Z_u};
		_zSliceMap[atom][centre] = {std::max(-R_B, a.z + Z_l - b.z), std::min(R_B, a.z + Z_u - b.z)};

		std::cout << "d_AB: " << d_AB << " R_A: " << R_A << " R_B: " << R_B << " Z_b: " << Z_b << std::endl;
	}
}

std::set<Atom *> ContactSheet::atomsNear(const AtomPosMap &posMap, Atom *centre,
																				 float radius)
{
	std::set<Atom *> nearAtomsSet;
  for (auto &atom : posMap)
	{
		if (sqlength(atom.first->derivedPosition() - centre->derivedPosition()) 
				<= radius*radius && atom.first != centre)
		{
			nearAtomsSet.insert(atom.first);
		}
	}
	return nearAtomsSet;
}
