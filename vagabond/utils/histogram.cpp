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

#include "histogram.h"

void get_histogram_limits(const std::vector<float> &vals, Histogram &hist)
{
	for (const float &val : vals)
	{
		hist.max = std::max(val, hist.max);
		hist.min = std::min(val, hist.min);
	}
}

void histogram(const std::vector<float> &vals, Histogram &hist)
{
	get_histogram_limits(vals, hist);

	std::vector<int> &freqs = hist.freqs;
		
	for (const float &val : vals)
	{
		int bin = (val - hist.min + hist.step / 2) / hist.step;
		if (freqs.size() < bin + 1)
		{
			freqs.resize(bin + 1);
		}

		freqs[bin]++;
		if (freqs.at(bin) > hist.highest)
		{
			hist.highest = freqs.at(bin);
		}
	}

	for (const int &freq : hist.freqs)
	{
		if (freq < hist.lowest)
		{
			hist.lowest = freq;
		}

	}
}

