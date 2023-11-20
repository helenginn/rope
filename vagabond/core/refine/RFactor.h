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

#ifndef __vagabond__RFactor__
#define __vagabond__RFactor__

#include <cmath>
#include <vector>
class Diffraction;

class RFactor
{
public:
	/* data must match model dimensions.
	 * data must be the source of binning information.
	 * @param bins number of bins in total, matching data binning info.
	 */
	RFactor(Diffraction *data, Diffraction *model, int bin_count);

	float operator()(int bin = -1);
private:
	void calculate();

	Diffraction *_data = nullptr;
	Diffraction *_model = nullptr;
	
	struct RunningTotal
	{
		float sum_data_minus_model = 0;
		float sum_data = 0;
		int total = 0;
		
		float operator()()
		{
			return sum_data_minus_model / sum_data;
		}
		void add(const float &data, const float &model)
		{
			sum_data_minus_model += fabs(data - model);
			sum_data += fabs(data);
			total++;
		}
	};

	std::vector<RunningTotal> _bins;
	RunningTotal _overall;
	
	bool _done = false;
};

#endif
