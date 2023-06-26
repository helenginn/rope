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
		}
	};
}


void AltersNetwork::bindPointGradients(int pidx, std::vector<Parameter *> &params)
{
	SpecificNetwork *sn = _specified;
	
	_getPoints = [pidx, sn, params](std::vector<float> &values)
	{
		values.clear();
		for (size_t i = 0; i < params.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;
			if (idx < 0) continue;

			for (int d = 0; d < bonds->n(); d++)
			{
				const Floats &grads = bonds->get_gradients(pidx, d);
				values.push_back(grads[idx]);
			}
		}
	};

	_setPoints = [pidx, sn, params](const std::vector<float> &src)
	{
		int n = 0;
		for (size_t i = 0; i < params.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;
			if (idx < 0) continue;

			for (int d = 0; d < bonds->n(); d++)
			{
				const float &new_val = src[n + d];
				Floats &dest = bonds->get_gradients(pidx, d);
				dest[idx] = new_val;
			}
			
			n += bonds->n();
		}
	};
}

void AltersNetwork::bindPointValues(int pidx, std::vector<Parameter *> &params)
{

	SpecificNetwork *sn = _specified;
	
	_getPoints = [pidx, sn, params](std::vector<float> &values)
	{
		values.clear();
		for (size_t i = 0; i < params.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;

			if (idx < 0) continue;

			const Floats &vals = bonds->get_value(pidx);
			values.push_back(vals[idx]);
		}
	};

	_setPoints = [pidx, sn, params](const std::vector<float> &src)
	{
		for (size_t i = 0; i < src.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;

			if (idx < 0) continue;

			const float &new_val = src[i];
			Floats &dest = bonds->get_value(pidx);
			dest[idx] = new_val;
		}
	};
}
