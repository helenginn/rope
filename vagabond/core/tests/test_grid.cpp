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

#include <vagabond/utils/include_boost.h>

#include <vagabond/core/ArbitraryMap.h>
#include <vagabond/core/MtzFile.h>
#include <vagabond/core/matrix_functions.h>
namespace tt = boost::test_tools;

ArbitraryMap *quickMap()
{
	int nx = 10; int ny = 10; int nz = 10;
	ArbitraryMap *map = new ArbitraryMap();
	map->setDimensions(nx, ny, nz);

	glm::mat3x3 uc = mat3x3_from_unit_cell(10, 10, 10, 90, 90, 90);
	map->setRealMatrix(uc);

	return map;
}

BOOST_AUTO_TEST_CASE(test_grid_adds_other)
{
	ArbitraryMap *map = quickMap();
	ArbitraryMap *second = quickMap();
	
	*map += *second;
	map->element(0)[0] = 1;
	map->element(4)[0] = 1;
	
	MtzFile test("test");
	test.setMap(map);

	std::string str = test.write_to_string();
	std::cout << str[0] << std::endl;
}
