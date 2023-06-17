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

void AltersNetwork::bindPoint(int pidx, std::vector<Parameter *> &params)
{
	std::vector<Mapped<float> *> maps = parametersToMaps(_specified, params);

	_getPoints = [pidx, maps](std::vector<float> &values)
	{
		values.clear();
		for (size_t i = 0; i < maps.size(); i++)
		{
			const float &new_val = maps[i]->get_value(pidx);
			values.push_back(new_val);
		}
	};

	_setPoints = [pidx, maps](const std::vector<float> &values)
	{
		for (size_t i = 0; i < maps.size(); i++)
		{
			const float &new_val = values[i];
			maps[i]->alter_value(pidx, new_val);
		}
	};
}
