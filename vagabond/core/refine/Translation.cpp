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

#include "Translation.h"
using namespace rope;

Translation::Translation(int hyperDims)
{
	_hyperDims = hyperDims;
	setupMatrix(&_defaults, _hyperDims + 1, 3);
	setupMatrix(&_coordinates, _hyperDims + 1, 3);
	
	
	for (size_t j = 0; j < 3; j++)
	{
		for (size_t i = 0; i < _hyperDims; i++)
		{
			if (i == j)
			{
				_defaults[i][j] = 2.0;
			}
		}
	}
}

Translation::~Translation()
{
	freeMatrix(&_defaults);
	freeMatrix(&_coordinates);
}

GetVec3FromIdx Translation::translate(const IntToCoordGet &get_coord,
                                      const std::vector<float> &params) const
{
	int i = 0;
	for (const float &p : params)
	{
		_coordinates.vals[i] = p + _defaults.vals[i];
		i++;
	}

	GetVec3FromIdx tr = [this, get_coord](const int &idx) -> glm::vec3
	{
		Coord::Get get = get_coord(idx); // raw sampled coordinate

		glm::vec3 result{};
		
		for (size_t j = 0; j < _coordinates.cols; j++)
		{
			for (size_t i = 0; i < _coordinates.rows; i++)
			{
				float val = (i == _coordinates.rows - 1) ? 1 : get(i);
				result[j] += val * _coordinates[i][j];
			}
		}
		
		return result;
	};
	
	return tr;
}
