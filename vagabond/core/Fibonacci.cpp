// Vagabond
// Copyright (C) 2017-2018 Helen Ginn
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

#include "Fibonacci.h"

Fibonacci::Fibonacci()
{

}

Fibonacci::~Fibonacci()
{

}


void Fibonacci::generateLattice(int num, double radius)
{
	if (num % 2 == 0)
	{
		num += 1;
	}

	_points.clear();
	_points.reserve(num);
	
	int rnd = 1;
	double offset = 2. / (double)num;
	double increment = M_PI * (3.0 - sqrt(5));

	for (int i = 0; i < num; i++)
	{
		double y = (((double)i * offset) - 1) + (offset / 2);
		double r = sqrt(1 - y * y);

		double phi = (double)((i + rnd) % num) * increment;

		double x = cos(phi) * r;
		double z = sin(phi) * r;

		vec3 point = make_vec3(x * radius, y * radius, z * radius);

		_points.push_back(point);
	}
}

