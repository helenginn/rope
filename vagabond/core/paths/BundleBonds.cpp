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

#include "BundleBonds.h"
#include "../Atom.h"
#include "../BondSequence.h"
#include "../ResidueId.h"
#include <gemmi/elem.hpp>

BundleBonds::BundleBonds(BondSequence *seq, const float &frac)
{
	_frac = frac;
	size_t size = seq->blockCount();
	_positions.resize(size);
	int n = 0;

	for (AtomBlock &block : seq->blocks())
	{
		_positions[n].atom = block.atom;
		n++;
	}
}

void BundleBonds::operator+=(const std::pair<int, BondSequence *> &pair)
{
	int p = pair.first;
	int n = 0;
	for (AtomBlock &block : pair.second->blocks())
	{
		_positions[n].pos[p] = block.my_position();
		
		n++;
	}
	
	_received[p] = true;
}

auto make_gradients(BundleBonds *bb)
{
	const std::array<bool, 4> &available = bb->received();
	int count = 0;
	for (const bool &b : available)
	{
		count += (b ? 1 : 0);
	}

	return [&available, count](const glm::vec3 *&ps, int idx) -> glm::vec3
	{
		if (count == 4)
		{
			return idx ?  (ps[2] - ps[0]) : (ps[3] - ps[1]);
		}
		else if (count == 3 && available[0] == 0)
		{
			//right hand
			return idx ?  (ps[2] - ps[1]) : (ps[1] - ps[2]);
		}
		else
		{
			//left hand
			return idx ?  (ps[1] - ps[0]) : (ps[0] - ps[1]);
		}
	};
}

void BundleBonds::findCoefficients()
{
	auto get_gradients = make_gradients(this);

	for (FourPos &ps : _positions)
	{
		const glm::vec3 *ptr = &ps.pos[0];

		const glm::vec3 g0 = get_gradients(ptr, 0);
		const glm::vec3 g1 = get_gradients(ptr, 1);

		const glm::vec3 &f0 = ps.pos[1];
		const glm::vec3 &f1 = ps.pos[2];

		ps.coefficients[0] = g0 + g1;
		ps.coefficients[1] = -2.f * g0 - g1;
		ps.coefficients[2] = g0 + f1 - f0;
		ps.coefficients[3] = f0;

		if (ps.atom)
		{
			gemmi::Element ele = gemmi::Element(ps.atom->elementSymbol());
			float vdwRadius = ele.vdw_r();
			ps.radius = vdwRadius + _vdwAdd;
			ps.atomic_num = ele.atomic_number();
		}
	}
}

long double vdw_energy(const float &difflength,
                       const float &ar, // radius
                       const float &aan, const float &ban) // atomic num
{
	float d = ar;
	float ratio = d / difflength;

	long double to6 = ratio * ratio * ratio * ratio * ratio * ratio;
	long double to12 = to6 * to6;

	// to roughly match tables of epsilon found online
	float weight = (aan + ban) / 25;

	long double potential = (to12 - to6) * weight;
	return potential;
}

auto get_diff_from_coeffs(const std::array<glm::vec3, 4> &diff)
{
	return [&diff](const float &f)
	{
		glm::vec3 pdiff = {};
		pdiff = diff[0] * f*f*f + diff[1] * f*f + diff[2] * f + diff[3];
		return pdiff;
	};
};

std::function<long double(int p, int q, float dist)> BundleBonds::lookup()
{
	findCoefficients();

	auto job = [this](int p, int q, float dist) -> long double
	{
		if (dist >= 0)
		{
			return vdw_energy(dist, _positions[p].radius,
			                  _positions[p].atomic_num,
			                  _positions[q].atomic_num);
		}

		std::array<glm::vec3, 4> diff = _positions[q] - _positions[p];

		int i = 0;
		float sum[] = {0, 0, 0, 0};
		for (const glm::vec3 &v : diff)
		{
			sum[i] = v[0] + v[1] + v[2];
			i++;
		}

		// solve for closest point

		float &a = sum[0]; float &b = sum[1]; 
		float &c = sum[2];

		auto diff_from_coeffs = get_diff_from_coeffs(diff);

		auto energy_from_diff = [this, p, q](const glm::vec3 &posdiff)
		{
			return vdw_energy(glm::length(posdiff), _positions[p].radius,
			                  _positions[p].atomic_num,
			                  _positions[q].atomic_num);
		};

		glm::vec3 pos0 = _positions[q].pos[1] - _positions[p].pos[1];
		float e = energy_from_diff(pos0);

		if (b*b - 3*a*c < 0)
		{
			return e;
		}

		float sqrt_term = sqrt(b*b - 3*a*c);
		float sol0 = (-b - sqrt_term) / (3*a);
		float sol1 = (-b + sqrt_term) / (3*a);

		if (sol0 == sol0 && sol0 > 0 && sol0 < 1)
		{
			float e2 = energy_from_diff(diff_from_coeffs(sol0));
			if (e2 == e2)
			{
				e = std::max(e2, e);
			}
		}
		else if (sol1 == sol1 && sol1 > 0 && sol1 < 1)
		{
			float e2 = energy_from_diff(diff_from_coeffs(sol1));
			if (e2 == e2)
			{
				e = std::max(e2, e);
			}
		}

		return e;
	};

	return job;
}

const ResidueId &BundleBonds::id(int idx)
{
	return _positions[idx].atom->residueId();
}


bool BundleBonds::areSidechains(int p, int q)
{
	return (!_positions[p].atom->isMainChain() && 
	        !_positions[q].atom->isMainChain());

}

void BundleBonds::report(int p, int q)
{
	std::cout << _positions[p].atom->desc() << " ";
	std::cout << _positions[q].atom->desc() << std::endl;
}
