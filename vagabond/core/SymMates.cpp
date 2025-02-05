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

#include <gemmi/symmetry.hpp>
#include "matrix_functions.h"

#include "Atom.h"
#include "SymMates.h"
#include "AtomGroup.h"
#include "GroupBounds.h"

AtomGroup *SymMates::getSymmetryMates(AtomGroup *const &other, 
                                      const std::string &spg_name,
                                      const std::array<double, 6> &uc, 
                                      float distance)
{
	AtomGroup *total = new AtomGroup();
	if (spg_name.length() == 0)
	{
		return total;
	}
	
	const gemmi::SpaceGroup *spg = gemmi::find_spacegroup_by_name(spg_name);

	gemmi::GroupOps grp = spg->operations();
	glm::mat3x3 uc_mat = {};
	uc_mat = mat3x3_from_unit_cell(uc[0], uc[1], uc[2], uc[3], uc[4], uc[5]);
	glm::mat3x3 to_frac = glm::inverse(uc_mat);
	float distsq = distance * distance;
	
	GroupBounds bounds(other);
	glm::vec3 min = bounds.min - glm::vec3(5.f);
	glm::vec3 max = bounds.max + glm::vec3(5.f);
	
	auto outside_bounds = [min, max](const glm::vec3 &target) -> bool
	{
		for (int i = 0; i < 3; i++)
		{
			if (min[i] > target[i]) { return true; }
			if (max[i] < target[i]) { return true; }
		}
		return false;
	};

	auto close_to = [distsq](::Atom *atom, const glm::vec3 &target) -> bool
	{
		glm::vec3 second = atom->initialPosition();
		for (int i = 0; i < 3; i++)
		{
			if (fabs(second[i] - target[i]) > distsq)
			{
				return false;
			}
		}
		glm::vec3 diff = second - target;
		float lsq = glm::dot(diff, diff);
		return (lsq < distsq && lsq >= 1e-3);
	};


	auto add_symop_atom_if_nearby = 
	[grp, close_to, other, to_frac, uc_mat, total, outside_bounds]
	(::Atom *const &atom)
	{
		glm::vec3 pos = atom->initialPosition();
		glm::vec3 frac = to_frac * pos;
		glm::vec3 nearest_origin = {};

		for (int i = 0; i < 3; i++)
		{
			nearest_origin[i] = (int)floor(frac[i]);
		}

		frac -= nearest_origin;
		std::array<double, 3> f = {frac.x, frac.y, frac.z};

		for (int l = 0; l < grp.sym_ops.size(); l++)
		{
			gemmi::Op op = grp.get_op(l);
			if (op == gemmi::Op::identity())
			{
				continue;
			}

			std::array<double, 3> g = op.apply_to_xyz(f);
			glm::vec3 sym_pos = glm::vec3(g[0] + nearest_origin.x,
			                              g[1] + nearest_origin.y,
			                              g[2] + nearest_origin.z);

			for (int k = -1; k <= 2; k++)
			{
				for (int j = -1; j <= 2; j++)
				{
					for (int i = -1; i <= 2; i++)
					{
						glm::vec3 trial_frac = sym_pos + glm::vec3(i, j, k);
						glm::vec3 trial = uc_mat * trial_frac;
						if (outside_bounds(trial))
						{
							continue;
						}
						
						::Atom *near = other->find_by([close_to, trial]
						                              (::Atom *const &a)
						                              {return close_to(a, trial);});
						if (near)
						{
							::Atom *copy = new ::Atom(*atom);
							std::string note = ("rot " + std::to_string(l) + 
							                    " trans " + std::to_string(i) +
							                    " " + std::to_string(j) +
							                    " " + std::to_string(k));
							copy->setSymmetryCopyOf(atom, note);
							copy->setInitialPosition(trial);
							copy->setDerivedPosition(trial);
							total->add(copy);
							return;
						}
					}
				}
			}
		}
	};

	other->do_op(add_symop_atom_if_nearby);
	return total;
}

