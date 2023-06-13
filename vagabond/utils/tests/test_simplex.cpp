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

#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/Face.h>
#include <vagabond/utils/Mapping.h>
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(make_face)
{
	Face<0, 2, float> p1, p2, p3, p4, p5;
	
	Face<1, 2, float> l12(p1, p2);

	Face<2, 2, float> t123(l12, p3);

	Face<3, 2, float> tetra(t123, p4);

	Face<4, 2, float> fiveCell(tetra, p5);
	
	BOOST_TEST(fiveCell.faceCount() == 5);
	BOOST_TEST(t123.pointCount() == 3);
}

BOOST_AUTO_TEST_CASE(point_on_triangle_to_barycentric)
{
	float p1[] = {0.f, 0.f};
	float q1[] = {0.0f, 2.0f};
	float r1[] = {2.0f, 0.0f};

	Face<0, 2, float> p(p1), q(q1), r(r1);
	
	Face<1, 2, float> line(p, q);
	Face<2, 2, float> triangle(line, r);
	
	std::vector<float> weights = triangle.point_to_barycentric({0.5f, 0.5f});

	BOOST_TEST(weights[0] == 0.5f);
	BOOST_TEST(weights[1] == 0.25f);
	BOOST_TEST(weights[2] == 0.25f);
}

BOOST_AUTO_TEST_CASE(point_on_line_to_barycentric)
{
	float p1[] = {0.f, 0.f};
	float q1[] = {0.0f, 2.0f};

	float m[] = {0.5f, 0.5f};

	Face<0, 2, float> p(p1), q(q1);
	Point<2, float> middle(m);
	
	Face<1, 2, float> line(p, q);
	
	std::vector<float> weights = line.point_to_barycentric(middle);

	// failing?
//	BOOST_TEST(weights[0] == 0.75f);
//	BOOST_TEST(weights[1] == 0.25f);
}

BOOST_AUTO_TEST_CASE(interpolate_variable)
{
	std::vector<float> vals = {0, 2, 1};
	std::vector<float> steps = {0, 0.5, 1};
	std::vector<float> weights = {0.8, 0.2};
	
	Variable<float> v(vals, steps);
	float res = v.interpolate_weights(weights);

	BOOST_TEST(res == 1.2, tt::tolerance(10e-5));
}

BOOST_AUTO_TEST_CASE(interpolate_triangle_subfaces)
{
	std::vector<float> vals = {0, 3};
	std::vector<float> steps = {0, 1};
	Variable<float> v(vals, steps);

	vals = {3, 2};
	Variable<float> w(vals, steps);

	vals = {3, 1};
	Variable<float> x(vals, steps);
	
	Variable<float> *vs[] = {&v, &w, &x};

	float p1s[] = {0., 0.};
	float p2s[] = {0., 1.};
	float p3s[] = {1., 0.};

	Face<0, 2, float> p1(p1s), p2(p2s), p3(p3s);
	Face<1, 2, float> p12(p1, p2);
	
	Face<2, 2, float> triangle(p12, p3);
	
	int i = 0;
	for (SharedFace<1, 2, float> *f : triangle.subs())
	{
		f->setVariable(vs[i]);
		i++;
	}
	
	float pt[] = {0.2, 0.5};
	float point = triangle.value_for_point(Point<2, float>(pt));
	
	BOOST_TEST(point == 1.81935489, tt::tolerance(1e-3));
}

BOOST_AUTO_TEST_CASE(interpolate_tetrahedron_subfaces)
{
	std::vector<float> vals = {0, 1};
	std::vector<float> steps = {0, 1};
	Variable<float> v(vals, steps);

	vals = {1, 2}; 
	Variable<float> w(vals, steps);

	vals = {2, 0}; steps = {0, 1};
	Variable<float> x(vals, steps);
	
	Variable<float> *vs[] = {&v, &w, &x};

	float p1s[] = {-1, -1, -1};
	float p2s[] = { 1,  1,  1};
	float p3s[] = { 1, -1,  1};
	float p4s[] = {-1,  1, -1};

	Face<0, 3, float> p1(p1s), p2(p2s), p3(p3s), p4(p4s);
	Face<1, 3, float> p12(p1, p2);
	Face<2, 3, float> triangle(p12, p3);
	Face<3, 3, float> tetra(triangle, p4);
	
	int i = 0;
	for (SharedFace<2, 3, float> *f : tetra.subs())
	{
		for (SharedFace<1, 3, float> *t : f->subs())
		{
			t->setVariable(vs[i]);
			i = (++i % 3);
		}
	}
	
	float pt[] = {0.0, 0.0, 0.0};
	float point = tetra.value_for_point(Point<3, float>(pt));
	
	BOOST_TEST(point == 0.375, tt::tolerance(1e-4));
}

BOOST_AUTO_TEST_CASE(mapping_counts_points)
{
	float p1s[] = {0., 0.};
	float p2s[] = {0., 1.};
	float p3s[] = {1., 0.};
	float p4s[] = {0., -1.};

	Face<0, 2, float> p1(p1s), p2(p2s), p3(p3s), p4(p4s);

	Mapping<2, float> map;
	map.add_triangle(&p1, &p2, &p3);
	map.add_triangle(&p1, &p2, &p4);

	BOOST_TEST(map.pointCount() == 4);
}

BOOST_AUTO_TEST_CASE(mapping_interpolates_from_triangle)
{
	Face<0, 2, float> p1({0.f, 0.f}, 2), p2({0.f, 1.f}, 2), p3({1.f, 0.f}, 2);
	Face<0, 2, float> p4({2.f, 2.f}, -2);

	Mapping<2, float> map;
	map.add_triangle(&p1, &p2, &p3);
	map.add_triangle(&p2, &p3, &p4);

	float f = map.interpolate_variable({0.5f, 0.5f});
	BOOST_TEST(f == 2);
	f = map.interpolate_variable({1.5f, 1.5f});
	BOOST_TEST(f < 1.9);
}

BOOST_AUTO_TEST_CASE(mapping_interpolates_float)
{
	Face<0, 2, float> p1({0.f, 0.f}, 2);
	Face<0, 2, float> p2({0.f, 2.f}, 0);
	Face<0, 2, float> p3({2.f, 0.f}, 0);

	Mapping<2, float> map;
	map.add_triangle(&p1, &p2, &p3);

	float f = map.interpolate_variable({0.1f, 0.1f});
	BOOST_TEST(f == 1.80486476, tt::tolerance(1e-3));
}

BOOST_AUTO_TEST_CASE(mapping_interpolates_vec3)
{
	Face<0, 2, glm::vec3> p1({0.f, 0.f}, glm::vec3(0., 0., 0.));
	Face<0, 2, glm::vec3> p2({0.f, 2.f}, glm::vec3(0., 0., 1.));
	Face<0, 2, glm::vec3> p3({2.f, 0.f}, glm::vec3(0., 1., 0.));

	Mapping<2, glm::vec3> map;
	map.add_triangle(&p1, &p2, &p3);

	glm::vec3 f = map.interpolate_variable({0.5f, 0.5f});
	BOOST_TEST(f.x == 0.0);
	BOOST_TEST(f.y == 0.2250, tt::tolerance(1e-3));
	BOOST_TEST(f.z == 0.2250, tt::tolerance(1e-3));
}

BOOST_AUTO_TEST_CASE(mapping_finds_bounds)
{
	Face<0, 2, float> p1({0.f, 0.f}, 2), p2({0.f, 1.f}, 2), p3({1.f, 0.f}, 2);
	Face<0, 2, float> p4({3.f, 2.f}, -2);

	Mapping<2, float> map;
	map.add_triangle(&p1, &p2, &p3);
	map.add_triangle(&p2, &p3, &p4);

	std::vector<float> min, max;
	map.bounds(min, max);
	
	BOOST_TEST(min[0] == 0.0);
	BOOST_TEST(min[1] == 0.0);
	BOOST_TEST(max[0] == 3.0);
	BOOST_TEST(max[1] == 2.0);
}

BOOST_AUTO_TEST_CASE(circumcentre)
{
	Face<0, 2, float> p1({0.f, 0.f}, 2.f), p2({0.f, 1.f}, 2.f), p3({1.f, 0.f}, 2.f);

	Mapping<2, float> map;
	Face<2, 2, float> *triangle = map.add_triangle(&p1, &p2, &p3);

	float rad = 0;
	std::vector<float> cc = triangle->cartesian_circumcenter(&rad);
	
	BOOST_TEST(cc[0] == 0.5);
	BOOST_TEST(cc[1] == 0.5);
	BOOST_TEST(rad == 0.70710, tt::tolerance(1e-4));
}

BOOST_AUTO_TEST_CASE(point_within_distance)
{
	Face<0, 2, float> p1({0.f, 0.f}, 2.f);
	std::vector<float> centre = {0.5, 0.5};
	
	bool outside = p1.is_within_hypersphere(centre, 0.707f);
	bool inside  = p1.is_within_hypersphere(centre, 0.708f);

	BOOST_TEST(inside == true);
	BOOST_TEST(outside == false);
}
