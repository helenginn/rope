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

#include "Recombine.h"

Recombine::Recombine(Diffraction *data, Diffraction *model)
{
	_data = data;
	_model = model;

	if (_data->nn() != _model->nn())
	{
		std::cout << "Warning: mismatched n for data/model!" << std::endl;
	}

	_combined = new Diffraction(_data);
}

Diffraction *Recombine::operator()()
{
	auto recombine = [this](long idx)
	{
		float fo = _data->element(idx).amplitude();
		float fc = _model->element(idx).amplitude();
		float ph = _model->element(idx).phase();

		float amp = 2 * fo - fc;
		_combined->element(idx).addAmplitudePhase(amp, ph);
	};
	
	_combined->do_op_on_each_1d_index(recombine);

	return _combined;
}
