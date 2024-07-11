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

#ifndef __vagabond__BundleBonds__
#define __vagabond__BundleBonds__

#include <functional>
#include <vector>
#include <array>
#include <set>
#include <vagabond/utils/glm_import.h>

class BondSequence;
struct ResidueId;
class Atom;

/* a bundle of three/four sets of points which can be used to make 
 * quadratic or cubic fit splines.*/

class BundleBonds
{
public:
	BundleBonds(BondSequence *seq, const float &frac);
	void operator+=(const std::pair<int, BondSequence *> &pair);
	
	std::function<long double(int p, int q, float dist)> lookup();
	
	const float &frac() const
	{
		return _frac;
	}
	
	bool recordContacts()
	{
		return _recordContacts;
	}

	void setRecordContacts(bool rc)
	{
		_recordContacts = rc;
	}
	
	const ResidueId &id(int idx);
	
	bool areSidechains(int p, int q);
	
	void report(int p, int q);
	
	const std::array<bool, 4> &received() const
	{
		return _received;
	}
private:
	void findCoefficients();

	struct FourPos
	{
		Atom *atom = nullptr;
		glm::vec3 pos[4] = {};
		std::array<glm::vec3, 4> coefficients = {}; // x^3 + x^2 + x + c
		float radius;
		float epsilon;
		
		std::array<glm::vec3, 4> operator-(const FourPos &other) const
		{
			std::array<glm::vec3, 4> coeffs = coefficients;
			for (int i = 0; i < 4; i++)
			{
				coeffs[i] -= other.coefficients[i];
			}
			return coeffs;
		}
	};

	bool _recordContacts = false;
	float _frac = 0;
	std::vector<FourPos> _positions;
	std::array<bool, 4> _received;
};

#endif
