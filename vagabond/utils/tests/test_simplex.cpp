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

#include <vagabond/utils/Face.h>
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(make_face)
{
	Face<0, 2> p1, p2, p3, p4, p5;
	
	Face<1, 2> l12(p1, p2);

	Face<2, 2> t123(l12, p3);

	Face<3, 2> tetra(t123, p4);

	Face<4, 2> fiveCell(tetra, p5);
	
	BOOST_TEST(fiveCell.faceCount() == 5);
	BOOST_TEST(t123.pointCount() == 3);
}

BOOST_AUTO_TEST_CASE(point_on_triangle_to_barycentric)
{
	float p1[] = {0.f, 0.f};
	float q1[] = {0.0f, 2.0f};
	float r1[] = {2.0f, 0.0f};

	float m[] = {0.5f, 0.5f};

	Face<0, 2> p(p1), q(q1), r(r1);
	Point<2> middle(m);
	
	Face<1, 2> line(p, q);
	Face<2, 2> triangle(line, r);
	
	std::vector<float> weights = triangle.point_to_barycentric(middle);

	BOOST_TEST(weights[0] == 0.5f);
	BOOST_TEST(weights[1] == 0.25f);
	BOOST_TEST(weights[2] == 0.25f);
}

BOOST_AUTO_TEST_CASE(point_on_line_to_barycentric)
{
	float p1[] = {0.f, 0.f};
	float q1[] = {0.0f, 2.0f};

	float m[] = {0.5f, 0.5f};

	Face<0, 2> p(p1), q(q1);
	Point<2> middle(m);
	
	Face<1, 2> line(p, q);
	
	std::vector<float> weights = line.point_to_barycentric(middle);

	BOOST_TEST(weights[0] == 0.75f);
	BOOST_TEST(weights[1] == 0.25f);
}

BOOST_AUTO_TEST_CASE(interpolate_variable)
{
	std::vector<float> vals = {0, 2, 1};
	std::vector<float> steps = {0, 0.5, 1};
	std::vector<float> weights = {0.8, 0.2};
	
	Variable v(vals, steps);
	float res = v.interpolate_weights(weights);

	BOOST_TEST(res == 1.2, tt::tolerance(10e-5));
}

BOOST_AUTO_TEST_CASE(interpolate_subfaces)
{
	std::vector<float> vals = {0, 1};
	std::vector<float> steps = {0, 1};
	Variable v(vals, steps);

	vals = {1, 2}; steps = {0, 1};
	Variable w(vals, steps);

	vals = {2, 0}; steps = {0, 1};
	Variable x(vals, steps);
	
	Variable *vs[] = {&v, &w, &x};

	float p1s[] = {0., 0.};
	float p2s[] = {0., 1.};
	float p3s[] = {1., 0.};

	Face<0, 2> p1(p1s), p2(p2s), p3(p3s);
	Face<1, 2> p12(p1, p2);
	
	Face<2, 2> triangle(p12, p3);
	
	int i = 0;
	for (SharedFace<1, 2> *f : triangle.subs())
	{
		f->setVariable(vs[i]);
		i++;
	}
	
	float pt[] = {0.5, 0.5};
	float point = triangle.value_for_point(Point<2>(pt));
	
	BOOST_TEST(point == 0.5);
}
