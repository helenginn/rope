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

#include "AddTogether.h"
#include "../grids/Diffraction.h"

Diffraction *AddTogether::operator()()
{
	Diffraction *result = new Diffraction(_first);

	auto add_other = [this, result](long idx)
	{
		float amp = _second->element(idx).amplitude();
		float ph = _second->element(idx).phase();

		result->element(idx).addAmplitudePhase(amp, ph);
	};
	
	result->do_op_on_each_1d_index(add_other);
	return result;
}
