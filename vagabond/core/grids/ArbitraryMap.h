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

#ifndef __vagabond__ArbitraryMap__
#define __vagabond__ArbitraryMap__

#include <fftw3.h>
#include "TransformedGrid.h"
#include "FFT.h"

class Diffraction;
class AtomMap;

class ArbitraryMap : 
virtual public TransformedGrid<fftwf_complex>,
virtual public FFT<fftwf_complex>
{
public:
	friend AtomMap;
	ArbitraryMap();
	ArbitraryMap(Diffraction &diff);
	ArbitraryMap(OriginGrid &other);
	ArbitraryMap(const ArbitraryMap &other);
	ArbitraryMap(const std::string &filename);
	
	ArbitraryMap &operator+=(const ArbitraryMap &other);
	ArbitraryMap &operator-=(const ArbitraryMap &other);
	ArbitraryMap &operator/=(const ArbitraryMap &other);
	ArbitraryMap &operator*=(const ArbitraryMap &other);

	void setupFromDiffraction();
	
	const std::string &spaceGroupName() const
	{
		return _spgName;
	}

	virtual void populatePlan(FFT<fftwf_complex>::PlanDims &dims);
	

	virtual float realValue(glm::vec3 real)
	{
		return interpolate(real);
	}
	
	void setReal(const long &i, const float &val)
	{
		_data[i][0] = val;
	}

	virtual float elementValue(long i) const
	{
		return _data[i][0];
	}
	

	void clear()
	{
		do_op_on_each_element([](fftwf_complex &ele)
		{
			ele[0] = 0; ele[1] = 0;
		});
	}

	bool withinReal(int i, int j, int k)
	{
		if (i >= nx() || i < -nx())
		{
			return false;
		}

		if (j >= ny() || j < -ny())
		{
			return false;
		}

		if (k >= nz() || k < -nz())
		{
			return false;
		}

		return true;
	}

	std::string spaceGroupName()
	{
		return _spgName;
	}
	
	virtual void multiply(float scale);
	void applySymmetry(const std::string &spg_name = "");
private:
	Diffraction *_diff = nullptr;

	std::string _spgName;
};

#endif
