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

ContactSheet::ContactSheet()
{
}

void ContactSheet::updateSheet(AtomPosMap &newPositions)
{
	// update any existing scratch calculations...
	//loop through newPositions and update _posMap:
	_posMap = newPositions;
}

void ContactSheet::calculateZSliceMap(Atom * centre, std::set<Atom *> nearAtoms)
{
	for (const auto &atom : nearAtoms)
	{
		float R_A, R_B, d_AB, x_a, x_b, y_a, y_b, z_a, z_b, Z_b, Z_c, Z_l, Z_u, r_b_sq, delta;

		R_A = getVdWRadius(centre);
		R_B = getVdWRadius(atom);
		d_AB = glm::length(centre->derivedPosition() - atom->derivedPosition());
		x_a = centre->derivedPosition().x;
		x_b = atom->derivedPosition().x;
		y_a = centre->derivedPosition().y;
		y_b = atom->derivedPosition().y;
		z_a = centre->derivedPosition().z;
		z_b = atom->derivedPosition().z;
		Z_b = z_b - z_a;
		r_b_sq = (x_a - x_b)*(x_a - x_b) + (y_a - y_b)*(y_a - y_b);
		Z_c = 0.5 * Z_b * (((R_A*R_A - R_B*R_B) / (d_AB*d_AB)) - 1);
		delta = sqrt(r_b_sq * (((R_B*R_B)/(d_AB*d_AB)) - ((Z_c*Z_c)/(Z_b*Z_b))));
		Z_l = (r_b_sq*(Z_c/Z_b) + delta*Z_b) > 0 ? Z_c - delta : - R_A;
		Z_u = (r_b_sq*(Z_c/Z_b) - delta*Z_b) <= 0 ? R_A : Z_c + delta;
		_zSliceMap[centre][atom] = {Z_l, Z_u};
		_zSliceMap[atom][centre] = {Z_l - Z_b, Z_u - Z_b};
	}
}

std::set<Atom *> ContactSheet::atomsNear(Atom *centre, float radius)
{
	// replace empty set with nearby atoms
	std::set<Atom *> nearAtomsSet;
  for (auto &atom : _posMap)
	{
		if (glm::length(atom.second.ave - centre->derivedPosition()) <= radius && atom.first != centre)
		{
			nearAtomsSet.insert(atom.first);
		}
	}
	return nearAtomsSet;
}
