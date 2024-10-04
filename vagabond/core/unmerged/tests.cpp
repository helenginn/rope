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

#include "TraditionalMerge.h"
#include "WeirdMerge.h"
#include "StillImage.h"
#include "Readings.h"
#include <vagabond/utils/FileReader.h>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	std::string data = get_file_contents("lysozyme_run7_unmerged.txt");
	char *ptr = &data[0];
	std::cout << "Data size: " << data.size() << std::endl;
	TraditionalMerge merge;
	std::vector<StillImage> _images;
	std::vector<StillImage> unique;
	
	int n = 0;
	while (n < 15000)
	{
		StillImage image;
		try
		{
			image = StillImage(ptr);
			_images.push_back(image);
		}
		catch (const char *msg)
		{
			break;
		}

		n++;
	}
	
	for (int i = 0; i < _images.size(); i++)
	{
		bool dupl = false;
		for (int j = i + 1; j < i + 50 && j < _images.size(); j++)
		{
			int count = _images[i].number_in_common(_images[j]);
			if (count == _images[i].size() && count > 0)
			{
				dupl = true;
				break;
			}
		}
		
		if (!dupl)
		{
			unique.push_back(_images[i]);
			merge += unique.back();
		}
	}
	
	merge("traditional.mtz");

	std::cout << "Now: " << unique.size() << " images" << std::endl;
	
	Readings readings;
	int tot = 0;
	n = 0;

	for (const StillImage &image : unique)
	{
		std::vector<Locality> localities = image.localities();

		for (const Locality &locality : localities)
		{
			readings += locality;
			tot += locality.size();
		}
		
		n++;
	}
	
	std::cout << "Readings: " << readings.size() << std::endl;
	std::cout << "Ave observations: " << tot / (float)readings.size() << std::endl;
	
	std::map<Reflection::HKL, Info> result = readings({5, 4, 2});
//	std::map<Reflection::HKL, Info> result = readings();
//	WeirdMerge weird(result);
//	weird("weird.mtz");
	return 0;
}
