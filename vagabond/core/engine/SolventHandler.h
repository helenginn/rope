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

#ifndef __vagabond__SolventHandler__
#define __vagabond__SolventHandler__

#include "engine/Handler.h"

class BondCalculator;

class SolventHandler : public Handler
{
public:
	SolventHandler(BondCalculator *calc);

	void setThreads(size_t threads)
	{
		_threads = threads;
	}
	
	void setMaskCount(const size_t masks)
	{
		_maskCount = masks;
	}
private:
	BondCalculator *_calculator = nullptr;
	
	int _threads = 2;
	int _maskCount = 2;
};

#endif
