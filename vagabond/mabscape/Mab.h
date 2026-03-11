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

#ifndef __vagabond__Mab__
#define __vagabond__Mab__

#include <vagabond/core/Model.h>
#include <vagabond/utils/OpSet.h>

class Chain;
class Metadata;
class AtomGroup;

struct Antigen
{
	std::string title{};
	Model model{};
	OpSet<std::string> entities;
	
	std::string validate();
};

class Antigens : public std::vector<Antigen>
{
public:
	std::string validate();

};

struct Competition
{
	std::string filename{};
	Metadata *metadata{};
};

struct ColourMap
{
	glm::vec3 colour_for(std::string ch)
	{
		if (mapping.count(ch) == 0)
		{
			return {0., 0., 0.};
		}
		else return mapping.at(ch);
	}

	std::map<std::string, glm::vec3> mapping{};
	void recalculate();
};

struct Mab
{
	Antigens antigens{};
	Competition competition{};
	ColourMap colours{};
};

#endif
