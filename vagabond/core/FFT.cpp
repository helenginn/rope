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
FFT<T>::FFT()
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
		doFFT(1);
		_status = Real;
	}
	else
	{
		throw std::runtime_error("FFT status is Empty but asked to transform");
	}
}

#endif
