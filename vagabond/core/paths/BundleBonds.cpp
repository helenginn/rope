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
			return idx == 0 ? (ps[2] - ps[0]) : (ps[3] - ps[1]);
		}
		else if (count == 3 && available[0] == 0)
		{
			//right hand
			return idx == 0 ? (ps[3] - ps[1]) : (ps[1] - ps[2]);
		}
		else
		{
			//left hand
			return idx == 1 ? (ps[1] - ps[0]) : (ps[0] - ps[1]);
		}
	};
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

void BundleBonds::findCoefficients()
{
//	auto get_gradients = make_gradients(this);

	glm::mat3 mat = {{-1, +1, -1}, {1, 1, 1}, {8, 4, 2}};
	glm::mat3 inv = glm::transpose(glm::inverse(mat));

	for (FourPos &ps : _positions)
	{
		ps.get_controls(_received);

		std::array<glm::vec3, 4> &p = ps.controls;

		for (int i = 0; i < 3; i++)
		{
			glm::vec3 ys = {};
			ys[0] = p[0][i];
			ys[1] = p[2][i];
			ys[2] = p[3][i];
			ys -= p[1][i];

			glm::vec3 abc = inv * ys;

			for (int j = 0; j < 3; j++)
			{
				ps.coefficients[j][i] = abc[j];
			}
		}

		ps.coefficients[3] = p[1];

		if (ps.atom)
		{
			gemmi::Element ele = gemmi::Element(ps.atom->elementSymbol());
			float vdwRadius = ele.vdw_r();
			ps.radius = vdwRadius;
			ps.epsilon = 0.07;
			switch (ele.atomic_number())
			{
				case 1: ps.epsilon = 0.0037; break;
				case 6: ps.epsilon = 0.0205; break;
				case 7: ps.epsilon = 0.0407; break;
				case 8: ps.epsilon = 0.0502; break;
				case 16: ps.epsilon = 0.0600; break;
			}
		}
	}
}

long double vdw_energy(const float &difflengthsq,
                       const float &sigma, // radius
                       const float &epsilon) // atomic num
{
	float d = sigma;
	float ratio = (d * d) / difflengthsq;

	long double to6 = ratio * ratio * ratio;
	long double to12 = to6 * to6;

	// to roughly match tables of epsilon found online

	long double potential = (epsilon * to12 - 2 * epsilon * to6);
	return potential;
}

std::function<long double(int p, int q, float dist)> BundleBonds::lookup()
{
	findCoefficients();

	auto job = [this](int p, int q, float dist) -> long double
	{
		float diameter = (_positions[p].radius + _positions[q].radius) * 0.75;
		float epsilon = sqrt(_positions[p].epsilon * _positions[q].epsilon);

		if (dist >= 0)
		{
			return vdw_energy(dist * dist, diameter, epsilon);
		}

		std::array<glm::vec3, 4> diff = _positions[q] - _positions[p];

		auto polynomial = get_diff_from_coeffs(diff);
		auto sqlength = [&polynomial](float f)
		{
			glm::vec3 res = polynomial(f);
			return glm::dot(res, res);
		};

		float close = FLT_MAX;
		float bottom = 0; float bottom_score = sqlength(0);
		float top = 1; float top_score = sqlength(1);
		float best_score = (top_score > bottom_score ?
		                    bottom_score : top_score);
		float thresh = 0.05;
		
		while (fabs(top - bottom) > thresh)
		{
			float trial = (top + bottom) / 2;
			float score = sqlength(trial);
			bool replace_top = (top_score > bottom_score);
			
			if (best_score > score)
			{
				best_score = score;
			}

			(replace_top ? top_score : bottom_score) = score;
			(replace_top ? top : bottom) = trial;
		}

		float e = vdw_energy(best_score, diameter, epsilon);

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
