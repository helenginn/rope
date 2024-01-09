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

#include <vagabond/core/grids/ArbitraryMap.h>

float peak_height_for_b_factor(float b)
{
	ArbitraryMap map;
	map.setDimensions(12, 12, 12, false);
	map.makePlans();
	map.setRealMatrix(glm::mat3(6));
	map.setStatus(ArbitraryMap::Reciprocal);

	auto fill_values = [&map, b](int i, int j, int k)
	{
		float res = map.resolution(i, j, k);
		float inv_dsq = 1 / (res * res);
		float exponent = -b * inv_dsq;
		float val = exp(exponent);

		map.element(i, j, k)[0] = val;
	};
	
	map.do_op_on_centred_index(fill_values);
	map.fft();
	return map.element(0)[0];
}

BOOST_AUTO_TEST_CASE(bfactor_occupancy_equivalent)
{
	float second_b = 49.94;
	float first_b = 37.75;

	float first_peak = peak_height_for_b_factor(first_b);
	float second_peak = peak_height_for_b_factor(second_b);

	std::cout << second_peak / first_peak << std::endl;
}
