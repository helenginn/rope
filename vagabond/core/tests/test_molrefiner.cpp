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

#include <vagabond/core/MolRefiner.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(test_triangle_to_svd)
{
	Refine::Info info{};
	info.axes.resize(4);

	MolRefiner mr(nullptr, &info, 1, 1);
	int size = mr.parameterCount();
	
	BOOST_TEST(size == 14);

	std::vector<float> triangle = {0, 1, 0, 2, 1, 0, 3, 2, 1, 0};
	
	mr.triangle_to_svd(triangle);

	std::vector<float> target = {0, 1, 2, 3, 1, 0, 1, 2, 2, 1, 0, 1, 3, 2, 1, 0};
	
	int track = 0;
	for (size_t i = 0; i < mr._svd.u.rows; i++)
	{
		for (size_t j = 0; j < mr._svd.u.cols; j++)
		{
			BOOST_TEST(mr._svd.u[i][j] == target[track], tt::tolerance(0.01));
			track++;
		}
	}
}
