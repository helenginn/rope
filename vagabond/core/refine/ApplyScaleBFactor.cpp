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

#include "ApplyScaleBFactor.h"
#include "../grids/Diffraction.h"

Diffraction *ApplyScaleBFactor::operator()(const float &k, const float &b)
{
	auto b_factor = [this, k, b](int i, int j, int l)
	{
		long idx = _model->index(i, j, l);
		float res = _model->resolution(i, j, l);
		float amp = _model->element(idx).amplitude();
		
		float b_val = exp(-b / (4 * res * res));
		if (idx == 0) b_val = 1;
		float mult = k * b_val;
		amp = amp * mult * -1;
		
		if (amp < 0)
		{
			float phase = _model->element(idx).phase();
			_model->element(idx).setPhase(phase + 180.f);
			amp = -amp;
		}

		_model->element(idx).setAmplitude(amp);
	};
	
	_model->do_op_on_centred_index(b_factor);
	return _model;
}
