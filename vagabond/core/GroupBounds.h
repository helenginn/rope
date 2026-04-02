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

#ifndef __vagabond__GroupBounds__
#define __vagabond__GroupBounds__

#include "AtomGroup.h"

struct GroupBounds
{
	typedef std::function<glm::vec3(Atom *const &)> GetPosition;
	
	GroupBounds(AtomGroup *const &grp, GetPosition get_pos = {})
	: _atoms(grp->atomVector()), _getPos(get_pos)
	{
		calculate();
	}

	GroupBounds(const std::vector<Atom *> &atoms, GetPosition get_pos = {})
	: _atoms(atoms), _getPos(get_pos)
	{
		calculate();
	}
	
	void calculate()
	{
		if (_done)
		{
			return;
		}
		
		if (!_getPos) _getPos = [](Atom *a) { return a->initialPosition(); };

		min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
		max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (size_t i = 0; i < _atoms.size(); i++)
		{
			glm::vec3 pos = _getPos(_atoms.at(i));

			for (size_t j = 0; j < 3; j++)
			{
				min[j] = std::min(min[j], pos[j]);
				max[j] = std::max(max[j], pos[j]);
			}
		}

		if (_atoms.size() == 0)
		{
			min = glm::vec3(0.);
			max = glm::vec3(0.);
		}
		
		_done = true;
	}

	const std::vector<Atom *> &_atoms;
	glm::vec3 min{};
	glm::vec3 max{};
	bool _done = false;
	GetPosition _getPos{};
	
	bool worth_checking_interface_with(AtomGroup *other, 
	                                   float min_dist)
	{
		AtomGroup *compare = other;
		GroupBounds bounds_compare(compare);

		for (int i = 0; i < 3; i++)
		{
			glm::vec3 &min1 = min;
			glm::vec3 &max1 = bounds_compare.max;
			if (max1[i] < min1[i] - min_dist) return false;

			glm::vec3 &max2 = max;
			glm::vec3 &min2 = bounds_compare.min;
			if (max2[i] < min2[i] - min_dist) return false;
		}

		return true;
	}

	AtomGroup *atoms_from_other_group_within(AtomGroup *other, float max_dist)
	{
		AtomGroup *all = new AtomGroup();
		for (Atom *const &atom : other->atomVector())
		{
			const glm::vec3 &pos = _getPos(atom);
			
			// first check: within bounds
			bool skip = false;
			for (int i = 0; i < 3 && !skip; i++)
			{
				if (pos[i] < min[i] - max_dist) skip = true;
				if (pos[i] > max[i] + max_dist) skip = true;
			}
			
			if (skip) continue;

			auto close_enough = [&pos, max_dist](const glm::vec3 &other)
			{
				for (int i = 0; i < 3; i++)
				{
					if (pos[i] < other[i] - max_dist) return false;
					if (pos[i] > other[i] + max_dist) return false;
				}

				return true;
			};
			
			// now for those atoms close enough, we check possible
			// individual partners
			for (Atom *const &mine : _atoms)
			{
				const glm::vec3 &q = _getPos(mine);
				if (close_enough(q))
				{
					float length = glm::length(pos - q);
					if (length < max_dist)
					{
						*all += atom; // other atom
					}
				}
			}
		}

		return all;
	}
};

#endif
