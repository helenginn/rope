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

#ifndef __vagabond__AtomRecall__
#define __vagabond__AtomRecall__

#include "Atom.h"

inline float distance_between(const std::vector<Atom *> &atoms)
{
	glm::vec3 first = atoms[0]->initialPosition();
	glm::vec3 last = atoms[1]->initialPosition();

	float dist = glm::length(first - last);
	return dist;
}

inline float angle_between(const std::vector<Atom *> &atoms)
{
	glm::vec3 middle = atoms[1]->initialPosition();
	glm::vec3 first = atoms[0]->initialPosition() - middle;
	glm::vec3 last = atoms[2]->initialPosition() - middle;
	first = glm::normalize(first);
	last = glm::normalize(last);

	float angle = rad2deg(glm::angle(first, last));
	return angle;
}

#endif
