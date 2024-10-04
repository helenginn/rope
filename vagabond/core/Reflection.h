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
#include <iostream>

struct Reflection
{
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

		bool operator<(const HKL &other) const
		{
			if (h != other.h) return h < other.h;
			if (k != other.k) return k < other.k;
			if (l != other.l) return l < other.l;
			return false;
		}
		
		bool operator==(const HKL &other) const
		{
			return (h == other.h && k == other.k && l == other.l);
		}

		bool near(const HKL &other, int tol = 1) const
		{
			return (abs(other.h - h) <= tol && abs(other.k - k) <= tol &&
			        abs(other.l - l) <= tol);
		}

        HKL()
        {

        }

        HKL(int hh, int kk, int ll)
        {
            h = hh; k = kk; l = ll;
        }
    };

	HKL hkl{};
	bool free = false;
	int flag = 0;
	float f = 0;
	float sigf = 0;
	float phi = 0;
	
	Reflection() {}
	
	Reflection(int h, int k, int l, float fv = 0, float sigfv = 0,
	           bool fr = false, bool fl = 1, float ph = 0)
	{
		hkl.h = h;
		hkl.k = k;
		hkl.l = l;
		f = fv;
		sigf = sigfv;
		free = fr;
		flag = fl;
		phi = ph;
	}
};

inline std::ostream &operator<<(std::ostream &ss, const Reflection::HKL &hkl)
{
	ss << "(" << hkl.h << " " << hkl.k << " " << hkl.l << ")";
	return ss;
}

#endif

