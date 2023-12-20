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

#include "Rotation.h"
using namespace rope;

Rotation::Rotation(int hyperDims, glm::vec3 centre)
{
	_hyperDims = hyperDims;
	_centre = centre;
	setupMatrix(&_coordinates, _hyperDims, 4);

}

Rotation::~Rotation()
{
	freeMatrix(&_coordinates);
}

rope::GetVec3FromIdx Rotation::rotate(const rope::IntToCoordGet &get_coord,
                                      const std::vector<float> &params) const
{
	int i = 0;
	for (const float &p : params)
	{
		_coordinates.vals[i] = p;
		i++;
	}

	GetVec3FromIdx rot = [this, get_coord](const int &idx, 
	                                      const glm::vec3 &v) -> glm::vec3
	{
		Coord::Get get = get_coord(idx); // raw sampled coordinate

		glm::vec3 from_centre = v - _centre;

		float raw[] = {0.f, 0.f, 0.f, 0.f};
		
		for (size_t j = 0; j < _coordinates.cols; j++)
		{
			for (size_t i = 0; i < _coordinates.rows; i++)
			{
				float val = get(i);
				raw[j] += val * _coordinates[i][j];
			}
		}
		
		for (size_t i = 0; i < 3 && false; i++)
		{
			raw[i] = 1 / (1 + exp(-raw[i])) - 0.5;
		}
		
		glm::vec3 axis = glm::vec3(raw[0], raw[1], raw[2]);
		float angle = glm::length(axis) / 2.f;
		axis = glm::normalize(axis);
		
		if (angle != angle) angle = 0;
		if (axis.x != axis.x) axis = {1, 0, 0};
		
		glm::mat3x3 rot = glm::mat3x3(glm::rotate(glm::mat4(1.f), angle, axis));
		
		glm::vec3 rotated = rot * from_centre;
		rotated += _centre;
		rotated -= v;

		return rotated;
	};
	
	return rot;

}
