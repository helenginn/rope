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

#include "WeirdMerge.h"

#define GEMMI_WRITE_IMPLEMENTATION
#include <gemmi/mtz.hpp>

void WeirdMerge::operator()(const std::string &filename)
{
	gemmi::Mtz mtz = gemmi::Mtz(true);
	mtz.spacegroup = gemmi::find_spacegroup_by_name("P 43 21 2");

	gemmi::UnitCell uc(79, 79, 38, 90, 90, 90);
	mtz.set_cell_for_all(uc);
	mtz.add_dataset("dataset");

	mtz.add_column("I", 'J', 0, -1, false);
	
	std::vector<float> data;
	data.reserve(_data.size() * 4);
	for (auto it = _data.begin(); it != _data.end(); it++)
	{
		data.push_back(it->first.h);
		data.push_back(it->first.k);
		data.push_back(it->first.l); 
		
		float merged = it->second.intensity;
		float weights = 1;

		data.push_back(merged); 
	}

	mtz.set_data(&data[0], data.size());
	mtz.write_to_file(filename);
}
