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

struct GroupBounds
{
	GroupBounds(AtomGroup *const &grp) : _atoms(grp->atomVector())
	{
		calculate();
	}

	GroupBounds(const std::vector<Atom *> &atoms) : _atoms(atoms)
	{
		calculate();
	}
	
	void calculate()
	{
		if (_done)
		{
			return;
		}

		min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
		max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (size_t i = 0; i < _atoms.size(); i++)
		{
			glm::vec3 pos = _atoms.at(i)->initialPosition();

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
};

#endif
