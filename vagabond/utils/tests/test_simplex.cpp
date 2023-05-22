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
#include <cmath>
#include <algorithm>

#define private public // evil but delicious
#define protected public // evil but delicious

#include <vagabond/utils/Simplex.h>
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(make_simplex)
{
	float p[] = {0.05f, 0.7f};
	Face<0, 2> p1(p), p2, p3, p4, p5;
	
	Face<1, 2> l12(p1, p2);

	Face<2, 2> t123(l12, p3);

	Face<3, 2> tetra(t123, p4);
	std::cout << tetra << std::endl;

	Face<4, 2> fiveCell(tetra, p5);
	
	BOOST_TEST(t123.faceCount() == 3);
	BOOST_TEST(tetra.faceCount() == 4);
	BOOST_TEST(fiveCell.faceCount() == 5);
}
