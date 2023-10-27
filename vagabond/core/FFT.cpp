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

#ifndef __vagabond__FFT_cpp__
#define __vagabond__FFT_cpp__

#include "FFT.h"

template <class T>
std::list<typename FFT<T>::PlanDims> FFT<T>::_plans;

template <class T>
FFT<T>::FFT(int nx, int ny, int nz) : Grid<T>(nx, ny, nz)
{
	makePlans();
}

template <class T>
FFT<T>::FFT() : Grid<T>()
{
	makePlans();
}

template <class T>
void FFT<T>::fft()
{
	if (_status == Real)
	{
		doFFT(1);
		_status = Reciprocal;
	}
	else if (_status == Reciprocal)
	{
		doFFT(-1);
		_status = Real;
	}
	else
	{
		throw std::runtime_error("FFT status is Empty but asked to transform");
	}
}

template <class T>
void FFT<T>::doFFT(int dir)
{
	if (dir > 0)
	{
		fftwf_execute_dft(_plan.forward,
		                  this->_planStart,
		                  this->_planStart);
	}
	else
	{
		fftwf_execute_dft(_plan.backward, 
		                  this->_planStart,
		                  this->_planStart);
		
		this->multiply(1 / (float)this->nn());
	}
}

template <class T>
void FFT<T>::createNewPlan()
{
	PlanDims plan{};
	plan.nx = this->nx();
	plan.ny = this->ny();
	plan.nz = this->nz();

	populatePlan(plan);
	
	_plan = plan;
}

template <class T>
typename FFT<T>::PlanDims *FFT<T>::findPlan(int nx, int ny, int nz) const
{
	for (auto it = _plans.begin(); it != _plans.end(); it++)
	{
		// not in use atm
		std::cout << it->nx << " " << it->ny << " " << it->nz << std::endl;
		if (it->nx == nx && it->ny == ny && it->nz == nz)
		{
			return &*it;
		}
	}

	return nullptr;
}

template <class T>
void FFT<T>::makePlans()
{
	PlanDims *plan = findPlan(this->nx(), this->ny(), this->nz());

	if (plan != nullptr)
	{
		_plan = *plan;
	}
	else
	{
		createNewPlan();
	}
}


#endif
