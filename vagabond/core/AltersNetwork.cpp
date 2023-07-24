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

void AltersNetwork::bindPointGradients(const std::vector<int> &idxs, 
                                       std::vector<Parameter *> &params)
{
	SpecificNetwork *sn = _specified;
	
	_getPoints = [idxs, sn, params](std::vector<float> &values)
	{
		values.clear();
		for (size_t i = 0; i < params.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;
			if (idx < 0) continue;
			
			for (int pidx : idxs)
			{
				for (int d = 0; d < bonds->n(); d++)
				{
					const Floats &grads = bonds->get_gradients(pidx, d);
					values.push_back(grads[idx]);
				}
			}
		}
	};

	_setPoints = [idxs, sn, params](const std::vector<float> &src)
	{
		int n = 0;
		for (size_t i = 0; i < params.size(); i++)
		{
			auto map_idx = sn->bondMappingFor(params[i]);
			Mapped<Floats> *bonds = map_idx.first;
			int idx = map_idx.second;
			if (idx < 0) continue;

			for (int pidx : idxs)
			{
				for (int d = 0; d < bonds->n(); d++)
				{
					const float &new_val = src[n + d];
					Floats &dest = bonds->get_gradients(pidx, d);
					dest[idx] = new_val;
				}

				n += bonds->n();
			}
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
