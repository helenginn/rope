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

#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include "StillImage.h"
#include <vagabond/utils/maths.h>

using HKL = Reflection::HKL;

void check_null(char *&str_ptr)
{
	if (*str_ptr == '\0')
	{
		throw "end";
	}
}

void wind_to_new_line(char *&str_ptr)
{
	while (*str_ptr != '\n')
	{
		str_ptr++;
		check_null(str_ptr);
	}

	str_ptr++;
	check_null(str_ptr);
}

int wind(char *&str_ptr)
{
	while (*str_ptr == '\t' || *str_ptr == ' ')
	{
		str_ptr++;
		check_null(str_ptr);
		if (*str_ptr == '\n')
		{
			return 1;
		}
	}
	
	if (*str_ptr == '\n')
	{
		return 1;
	}
	
	return 0;
}

std::vector<float> numbers_after_header(char *&str_ptr, const std::string &header)
{
	std::vector<float> numbers;
	if (strncmp(str_ptr, header.c_str(), header.length()) == 0)
	{
		str_ptr += header.length() + 1;
	}
	else
	{
		return {};
	}

	int end_of_line = 0;
	while (!end_of_line)
	{
		end_of_line = wind(str_ptr);
		if (end_of_line)
		{
			return numbers;
		}
		
		char *update;
		float num = strtof(str_ptr, &update);
		numbers.push_back(num);
		str_ptr = update;
	}

	return numbers;
}

int StillImage::checkLine(char *&str_ptr)
{
	std::vector<float> crystal = numbers_after_header(str_ptr, "Crystal");
	if (crystal.size() && _crystal >= 0)
	{
		// already assigned, meaning this is the next crystal. stop.
		return 0;
	}
	else if (crystal.size())
	{
		_crystal = crystal[0];
		return 1;
	}

	std::string header = "Flagged: yes";
	if (strncmp(str_ptr, header.c_str(), header.length()) == 0)
	{
		_flagged = true;
		return -1;
	}

	std::vector<float> r = numbers_after_header(str_ptr, " ");
	if (r.size() == 5 && !(r[0] == 0 && r[1] == 0 && r[2] == 0))
	{
		HKL hkl{(int)lrint(r[0]), (int)lrint(r[1]), (int)lrint(r[2])};
		Obs obs{hkl, {r[3], r[4]}};
		this->insert(obs);

		return 1;
	}

	return -1;
}

StillImage::StillImage(char *&str_ptr)
{
	while (true)
	{
		char *backup = str_ptr;
		int result = checkLine(str_ptr);
		if (result == -1)
		{
			wind_to_new_line(str_ptr);
		}
		else if (result == 0)
		{
			str_ptr = backup;
			break;
		}
	}
	
	if (_flagged)
	{
		this->clear();
	}
}

int StillImage::number_in_common(const StillImage &other)
{
	int count = 0;

	for (const Obs &obs : *this)
	{
		float mine = obs.info.intensity;
		if (other.count(obs))
		{
			count++;
		}
	}

	return count;
}

float StillImage::cc(const StillImage &other)
{
	std::vector<float> xs, ys, weights;
	for (const Obs &obs : *this)
	{
		float mine = obs.info.intensity;
		if (other.count(obs))
		{
			float yours = other.find(obs)->info.intensity;
			xs.push_back(mine);
			ys.push_back(yours);
			weights.push_back(other.find(obs)->info.weight * obs.info.weight);
		}
	}

	float cc = correlation(xs, ys, -FLT_MAX, &weights);
	cc *= (xs.size() - 1) / (float)xs.size();
	return cc;
}

std::vector<Locality> StillImage::localities() const
{
	std::vector<Locality> tmp;
	for (const Obs &obs : *this)
	{
		struct ObsFloat
		{
			const Obs *other;
			float dist;
			bool operator<(const ObsFloat &obs) const
			{
				return dist < obs.dist;
			}
		};

		Locality locality(obs);
		std::vector<ObsFloat> measurements;

		for (const Obs &other : *this)
		{
			ObsFloat entry{&other, sq_dist_from(obs, other)};
			measurements.push_back(entry);
		}

		std::sort(measurements.begin(), measurements.end(),

		[](const ObsFloat &a, const ObsFloat &b) { return a < b; });
		
		for (int i = 0; i < 2 && i < measurements.size(); i++)
		{
			locality += *measurements[i].other;
		}
		
		tmp.push_back(locality);
	}
	return tmp;
}
