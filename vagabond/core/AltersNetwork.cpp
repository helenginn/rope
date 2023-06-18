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

#include <vagabond/utils/Mapping.h>
#include "AltersNetwork.h"
#include "SpecificNetwork.h"

AltersNetwork::AltersNetwork(SpecificNetwork *sn, Mapped<float> *mapped,
                             std::atomic<bool> &stop, std::atomic<bool> &skip)
: _stop(stop), _skip(skip)
{
	_specified = sn;
	_mapped = mapped;
}

std::vector<Mapped<float> *> parametersToMaps(SpecificNetwork *sn, 
                                              std::vector<Parameter *> &params)
{
	std::vector<Mapped<float> *> maps;

	for (Parameter *p : params)
	{
		maps.push_back(sn->mapForParameter(p));
	}

	return maps;
}

void AltersNetwork::functionsForPositions(std::vector<Mapped<float> *> &maps,
                                          int pidx, AltersNetwork::Getter &getter,
                                          AltersNetwork::Setter &setter)
{
	getter = [pidx, maps](std::vector<float> &values)
	{
		for (size_t i = 0; i < maps.size(); i++)
		{
			if (maps[i] == nullptr) continue;
			std::vector<float> tmp = maps[i]->point_vector(pidx);
			values.reserve(values.size() + tmp.size());
			values.insert(values.end(), tmp.begin(), tmp.end());
		}
	};

	setter = [pidx, maps](const std::vector<float> &values)
	{
		// we know how many values there are: maps.size() - these are
		// already taken care of.
		
		int n = maps.size();
		for (size_t i = 0; i < maps.size(); i++)
		{
			if (maps[i] == nullptr) continue;
			// will get ignored after D in Point
			maps[i]->set_point_vector(pidx, &values[n]);
			n += maps[i]->n(); // n coordinates per point to set
			maps[i]->update(pidx);
		}
	};

}


void AltersNetwork::bindPoint(int pidx, std::vector<Parameter *> &params,
                              bool with_positions)
{
	std::vector<Mapped<float> *> maps = parametersToMaps(_specified, params);
	
	Getter get_coords; Setter set_coords;
	if (with_positions)
	{
		functionsForPositions(maps, pidx, get_coords, set_coords);
	}

	_getPoints = [pidx, maps, get_coords](std::vector<float> &values)
	{
		values.clear();
		for (size_t i = 0; i < maps.size(); i++)
		{
			if (maps[i] == nullptr) continue;
			const float &new_val = maps[i]->get_value(pidx);
			values.push_back(new_val);
		}
		if (get_coords)
		{
			get_coords(values);
		}
	};

	_setPoints = [pidx, maps, set_coords](const std::vector<float> &values)
	{
		for (size_t i = 0; i < maps.size(); i++)
		{
			if (maps[i] == nullptr) continue;
			const float &new_val = values[i];
			maps[i]->alter_value(pidx, new_val);
		}
		if (set_coords)
		{
			set_coords(values);
		}
	};
}
