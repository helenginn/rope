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

#ifndef __vagabond__GradientPath__
#define __vagabond__GradientPath__

#include <vagabond/utils/Vec3s.h>
#include <mutex>
#include <list>
#include "RTMotion.h"

class BondSequence;
class Parameter;
class Separation;
class PairwiseDeviations;

struct GradientTerm
{
	GradientTerm() = default;

	typedef std::function<Floats(const float &frac)> Weights;

	GradientTerm(const int &order, const float &f, const int &gidx,
	             const int &bidx, Parameter *const &p, const Weights &weights)
	{
		frac = f;
		b_idx = bidx;
		g_idx = gidx;
		param = p;
		_weights = weights;
		grads.resize(order);
	}
	
	void momentum(BondSequence *sequence, PairwiseDeviations *main,
	              Separation *sep);

	float frac = 0;
	int b_idx = 0; // block index
	int g_idx = 0; // gradient index
	Parameter *param = nullptr;
	Weights _weights;

	Floats grads;
};

struct GradientPath
{
	GradientPath()
	{
		mutex = new std::mutex();
	}
	
	void destroy()
	{
		delete mutex;
	}
	
	size_t size()
	{
		return motion_idxs.size();
	}
	
	void print();

	GradientPath &operator=(const GradientTerm &term)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		grads[term.g_idx].resize(term.grads.size());
		int n = 0;
		for (const float &f : term.grads)
		{
			grads[term.g_idx][n] += f / (float)term.grads.size();
			n++;
		}
		return *this;
	}

	std::mutex *mutex = nullptr;
	std::vector<Floats> grads;
	std::vector<int> motion_idxs;
	int ticket = 0;
};

#endif
