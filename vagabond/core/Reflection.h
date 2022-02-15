// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__Reflection__
#define __vagabond__Reflection__

#include <stdexcept>

struct HKL
{
	int h = 0;
	int k = 0; 
	int l = 0;
	
	int &operator[](int idx) 
	{
		if (idx == 0) return h;
		if (idx == 1) return k;
		if (idx == 2) return l;
		throw std::runtime_error("dimension over 3 accessing HKL");
	}
};

struct Reflection
{
	HKL hkl{};
	bool free = false;
	int flag = 0;
	float f = 0;
	float sigf = 0;
	
	Reflection() {}
	
	Reflection(int h, int k, int l)
	{
		hkl.h = h;
		hkl.k = k;
		hkl.l = l;
	}
};

#endif

