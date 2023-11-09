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

#include "RFactor.h"
#include "UtilitiesDiffraction.h"

RFactor::RFactor(Diffraction *data, Diffraction *model, int count)
{
	_data = data;
	_model = model;
	
	if (_data->nn() != _model->nn())
	{
		std::cout << "Warning: mismatched n for data/model!" << std::endl;
	}
	
	if (count >= 0)
	{
		_bins.resize(count);
	}
}

float RFactor::operator()(int bin)
{
	if (!_done)
	{
		calculate();
	}
	
	if (bin < 0)
	{
		return _overall();
	}
	
	if (bin > _bins.size())
	{
		return FLT_MAX;
	}

	return _bins[bin]();
}

void RFactor::calculate()
{
	auto mask = Utilities::mask_from_reference(_data);
	auto add_to_bins = [this, mask](long idx)
	{
		if (mask(idx))
		{
			return;
		}

		float fo = _data->element(idx).amplitude();
		float fc = _model->element(idx).amplitude();

		int cat = _data->element(idx).category;
		if (cat < 0 || (_bins.size() > 0 && cat > _bins.size()))
		{
			return;
		}

		_bins[cat].add(fo, fc);
		_overall.add(fo, fc);
	};

	_data->do_op_on_each_1d_index(add_to_bins);
	
	for (size_t i = 0; i < _bins.size(); i++)
	{
		std::cout << "bin " << i << ": " << _bins[i].total << 
		" -> " << _bins[i]() * 100 << "%" << std::endl;
	}
	
	_done = true;
}
