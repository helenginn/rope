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

#ifndef __vagabond__FFT__
#define __vagabond__FFT__

#include "TransformedGrid.h"

template <class T>
class FFT : public virtual Grid<T>
{
public:
	FFT();

	virtual void fft();
	virtual void makePlans() {}
	
	enum Status
	{
		Empty,
		Real,
		Reciprocal
	};
	
	void setStatus(Status status)
	{
		_status = status;
	}
protected:
	virtual void doFFT(int dir) = 0;
private:
	Status _status = Empty;

};

#include "FFT.cpp"

#endif
