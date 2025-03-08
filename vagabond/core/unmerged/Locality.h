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

#ifndef __vagabond__Locality__
#define __vagabond__Locality__

#include <map>
#include "Reflection.h"

struct Info
{
	float intensity;
	float weight;
};

struct Obs
{
	Reflection::HKL hkl;
	Info info;
	
	bool operator<(const Obs &other) const
	{
		return hkl < other.hkl;
	}
	
	bool near(const Obs &other) const
	{
		return hkl.is_near(other.hkl, 1);
	}
};

inline float sq_dist_from(const Reflection::HKL &hkl, 
                          const Reflection::HKL &other)
{
	float f = 0;
	f += (hkl.h - other.h) * (hkl.h - other.h);
	f += (hkl.k - other.k) * (hkl.k - other.k);
	f += (hkl.l - other.l) * (hkl.l - other.l);
	return f;
}

inline float sq_dist_from(const Obs &obs, const Obs &other)
{
	return sq_dist_from(obs.hkl, other.hkl);
}

class Locality : public std::map<Reflection::HKL, Info>
{
public:
	Locality(const Obs &central_obs)
	{
		_obs = central_obs;
	}
	
	float sum() const;

	void operator+=(const Obs &obs);
	
	float weight(const Reflection::HKL &hkl) const;
	
	const Reflection::HKL &hkl() const
	{
		return _obs.hkl;
	}
	
	const Info &info() const
	{
		return _obs.info;
	}
	
	std::map<Reflection::HKL, Info> operator()();
private:
	Obs _obs;

};

#endif
