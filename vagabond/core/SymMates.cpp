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

	// deliberately does not stop at the first (i, j, k, l) that matches -
	// a special position (e.g. sitting near a 2-/3-/4-fold axis) can have
	// several distinct symmetry-related copies genuinely nearby at once,
	// not just one, and all of them need registering as real mates for
	// the SymOp-keyed equivalence lookup (Atom::symmetryEquivalent()) to
	// have anything to find later.
	auto do_on_nearby_unit_cells = [grp]<typename Func>(const Func &func)
	{
		bool found = false;

		for (int k = -1; k <= 2; k++)
		{
			for (int j = -1; j <= 2; j++)
			{
				for (int i = -1; i <= 2; i++)
				{
					for (int l = 0; l < grp.sym_ops.size(); l++)
					{
						if (grp.get_op(l) == gemmi::Op::identity())
						{
							continue;
						}

						found |= func(i, j, k, l);
					}
				}
			}
		}

		return found;
	};
	
	auto make_do_sym_op = [grp, to_frac](int l)
	{
		return [grp, to_frac, l](const glm::vec3 &orig)
		{
			glm::vec3 frac = to_frac * orig;
			glm::vec3 nearest_origin = {};

			for (int i = 0; i < 3; i++)
			{
				nearest_origin[i] = (int)floor(frac[i]);
			}

			frac -= nearest_origin;
			std::array<double, 3> f = {frac.x, frac.y, frac.z};
			gemmi::Op op = grp.get_op(l);
			std::array<double, 3> g = op.apply_to_xyz(f);
			glm::vec3 rfrac = {g[0], g[1], g[2]};
			glm::vec3 sym_pos = glm::vec3(rfrac + nearest_origin);
			return sym_pos;
		};
	};

	auto add_symop_atom_if_nearby = 
	[close_to, other, uc_mat, total, outside_bounds,
	 do_on_nearby_unit_cells, make_do_sym_op]
	(::Atom *const &atom, glm::vec3 pos)
	{
		auto check_if_atom_is_near = 
		[pos, uc_mat, outside_bounds, other, close_to, atom, 
		 total, make_do_sym_op]
		(int i, int j, int k, int l)
		{
			auto do_frac_sym = make_do_sym_op(l);
			glm::vec3 sym_pos = do_frac_sym(pos);

			glm::vec3 trial_frac = sym_pos + glm::vec3(i, j, k);
			glm::vec3 trial = uc_mat * trial_frac;
			if (outside_bounds(trial))
			{
				return false;
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
				SymOp op{l, i, j, k};
				copy->setSymmetryCopyOf(atom, op, note);
				copy->setInitialPosition(trial);

				for (std::string c : atom->conformerList())
				{
					glm::vec3 other = atom->conformerPositions()[c].pos.ave;
					float b = atom->conformerPositions()[c].b;
					float occ = atom->conformerPositions()[c].occ;
					glm::vec3 fracsym = do_frac_sym(other) + glm::vec3(i, j, k);
					glm::vec3 sympos = uc_mat * fracsym;
					std::cout << "sympos: " << sympos << " near " <<
					pos << std::endl;
					copy->conformerPositions()[c].pos.ave = sympos;
					copy->conformerPositions()[c].occ = occ;
					copy->conformerPositions()[c].b = b;
				}

				copy->setDerivedPosition(trial);
				total->add(copy);
				return true;
			}

			return false;
		};

		return do_on_nearby_unit_cells(check_if_atom_is_near);
	};

	auto on_each_conf_pos = [] <typename Func>(const Func &func)
	{
		return [&func](::Atom *a)
		{
			for (std::string conformer : a->conformerList())
			{
				glm::vec3 pos = a->conformerPositions()[conformer].pos.ave;
				if (func(a, pos)) // when successful, stop
				{
					return;
				}
			}
		};
	};

	other->do_op(on_each_conf_pos(add_symop_atom_if_nearby));
	return total;
}

