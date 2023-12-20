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
#include "CentroidToPhase.h"
#include "grids/Diffraction.h"

Recombine::Recombine(Diffraction *data, Diffraction *model, const Type &type)
{
	_data = data;
	_model = model;
	_type = type;

	if (_data->nn() != _model->nn())
	{
		std::cout << "Warning: mismatched n for data/model!" << std::endl;
	}

	_combined = new Diffraction(_data);
	auto wipe = [this](long idx)
	{
		_combined->element(idx) = {};
	};
	_combined->do_op_on_each_1d_index(wipe);
}

// from: https://www.hindawi.com/journals/mpe/2012/124029/
float cumulative_gaussian(const float &x)
{
	float arctan = atan(37*x/294);
	float exponent = (179*x/23) - (111*arctan/2);
	float nom = tanh(exponent);
	float adjust = nom / 2 + 0.5;
	return adjust;
}

float proportion_amplitude_below_zero(const float &f, const float &sigma)
{
	float prop = -f / sigma;
	return cumulative_gaussian(prop);
}

Diffraction *Recombine::operator()()
{
	auto recombine = [this](long idx)
	{
		float fo = _data->element(idx).amplitude();
		float fc = _model->element(idx).amplitude();
		float ph = _model->element(idx).phase();

		float amp = 0;
		float diff = fabs(fo - fc);
		
		float fo_error = sqrt(fo) / fo;
		float diff_error = diff / fo;
		float standard_error = sqrt(fo_error * fo_error + diff_error * diff_error);
		float centroid = 0;

		int index = lrint(standard_error / 0.01);
		if (index < 314)
		{
			centroid = centroid_to_phase_stdev[index * 2 + 1];
		}
		
		switch (_type)
		{
			case Weighted:
			amp = centroid * fo;
			break;
			
			case Difference:
			amp = fo - fc;
			break;
			
			default:
			break;
		}
		
		if (amp != amp)
		{
			amp = 0;
		}
		
		// when Fc is very small, we need to propagate over error introduced
		// by phase sign error

		float prop = proportion_amplitude_below_zero(fc, diff);
		
		// we use this to reduce the size of our amplitude if Fc is near origin
		amp *= 1 - 2 * prop;

		_combined->element(idx).addAmplitudePhase(amp, ph);
	};
	
	_combined->do_op_on_each_1d_index(recombine);

	return _combined;
}
