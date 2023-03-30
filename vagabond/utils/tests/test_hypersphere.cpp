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

#define BOOST_TEST_MODULE utils
#include <vagabond/utils/include_boost.h>
#include <cmath>

#define private public // evil but delicious

#include <vagabond/utils/ProbDist.h>
#include <vagabond/utils/Hypersphere.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(simple_interpolation)
{
	ProbDist pd;
	pd.addProbability(0.f, 0.5);
	pd.addProbability(1.f, 1.5);

	float mid = pd.interpolate(0.5);

	BOOST_TEST(mid == 1.0, tt::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(simple_cumulative)
{
	ProbDist pd;
	pd.addProbability(0.0f, 0.5);
	pd.addProbability(0.5f, 0.2);
	pd.addProbability(1.0f, 1.5);

	float start = pd.cumulative(0.0);
	float middle = pd.cumulative(0.5);
	float end = pd.cumulative(1.0);

	BOOST_TEST(start == 0.0, tt::tolerance(0.001));
	BOOST_TEST(middle == 0.2916667, tt::tolerance(0.001));
	BOOST_TEST(end   == 1.0, tt::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(probability_inverse)
{
	ProbDist pd;
	pd.addProbability(0.0f, 0.5);
	pd.addProbability(0.5f, 0.2);
	pd.addProbability(1.0f, 1.5);

	float start = pd.cumulative(0.0);
	float end = pd.cumulative(1.0);
	float last = -1;

	for (float f = 0; f < 1.0; f += 0.1)
	{
		float next = pd.inverse_cumulative(f);
		BOOST_TEST(next > last);
		if (f > 0.05)
		{
			BOOST_TEST(next > f);
		}
		last = next;
	}
}

BOOST_AUTO_TEST_CASE(hypersphere_dimension)
{
	Hypersphere hs(3, 100);
	
	ProbDist pd = hs.setup_distribution_for_other(2);
	float last = -1;
	
	for (float a = 0; a < 2 * M_PI; a += 0.1)
	{
		float next = pd.cumulative(a);
		BOOST_TEST(next > last);
		last = next;
	}
}

BOOST_AUTO_TEST_CASE(check_first_primes)
{
	Hypersphere hs(5, 1);
	hs.preparePrimes();
	
	std::vector<int> five = {2, 3, 5, 7, 11};
	
	for (size_t i = 0; i < 5; i++)
	{
		BOOST_TEST(hs._primes[i] == five[i]);
	}
}

BOOST_AUTO_TEST_CASE(angle_to_cartesian)
{
	Hypersphere hs(5, 300);
	
	std::vector<float> angles = {1, 4, 3, 2, 5};
	std::vector<float> point = hs.angles_to_cartesian(angles);

	float sum = 0;
	for (float &p : point)
	{
		sum += p * p;
	}
	sum = sqrt(sum);
	
	BOOST_TEST(sum == 1.0, tt::tolerance(0.0001));
}


BOOST_AUTO_TEST_CASE(check_fibonacci)
{
	Hypersphere hs(4, 100);
	hs.prepareFibonacci();

	for (size_t i = 0; i < hs.count(); i++)
	{
		std::vector<float> point = hs.point(i);
		
		float sum = 0;
		for (float &p : point)
		{
			sum += p * p;
		}
		sum = sqrt(sum);

		BOOST_TEST(sum == 1.0, tt::tolerance(0.001));
	}
}


