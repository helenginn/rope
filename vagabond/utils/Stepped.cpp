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

#include "Stepped.h"
#include <iostream>

int Stepped::priorStep(float frac)
{
	if (_steps.size() == 0) return -1;

	int n = 0;
	while (_steps[n] < frac && n < _steps.size() - 1)
	{
		n++;
	}
	
	n--; // back one to get the pre-step
	
	if (n >= (int)_steps.size() - 1)
	{
		return -1;
	}

	return n;
}


