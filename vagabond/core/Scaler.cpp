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

#include <vagabond/utils/maths.h>
#include <vagabond/utils/Vec3s.h>
#include "UtilitiesDiffraction.h"
#include "Scaler.h"

Scaler::Scaler(Diffraction *const &data, Diffraction *const &model,
               const std::vector<float> &binning)
: _data(data), _model(model)
{
	if (_data->nn() != _model->nn())
	{
		std::cout << "Warning: mismatched n for data/model!" << std::endl;
	}

	_binning = binning;
}

template <typename Mask>
Floats averages_for_bins(Mask mask, Diffraction *reciprocal, int count)
{
	Floats averages(count, 0.f);
	std::vector<int> counts(count, 0);

	auto add_average = [&averages, &counts, mask, reciprocal](long idx)
	{
		if (mask(idx))
		{
			return;
		}

		float amp = reciprocal->element(idx).amplitude();
		int cat = reciprocal->element(idx).category;
		if (cat < 0 || amp != amp)
		{
			return;
		}

		averages[cat] += amp;
		counts[cat]++;
	};

	reciprocal->do_op_on_each_1d_index(add_average);
	
	return averages;
}

void scale(Diffraction *reciprocal, const Floats &corrections)
{
	auto multiply_through = [corrections, reciprocal](long idx)
	{
		float amp = reciprocal->element(idx).amplitude();
		int cat = reciprocal->element(idx).category;
		if (cat < 0 || amp != amp)
		{
			return;
		}

		float multiply = corrections[cat];
		reciprocal->element(idx).setAmplitude(amp * multiply);
	};

	reciprocal->do_op_on_each_1d_index(multiply_through);
}

void Scaler::operator()()
{
	float maxRes = _data->maxResolution();
	
	if (!_data->sliced())
	{
		_data->sliceIntoBins(_binning);
	}

	if (!_model->sliced())
	{
		_model->copyBinningFrom(_data);
	}

	auto mask = Utilities::mask_from_reference(_data);
	Floats corrections = averages_for_bins(mask, _data, _binning.size());
	Floats model_aves = averages_for_bins(mask, _model, _binning.size());
	
	corrections /= model_aves;
	
	scale(_model, corrections);
}
