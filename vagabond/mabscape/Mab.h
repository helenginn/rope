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

#include <list>
#include <vagabond/core/Model.h>
#include <vagabond/utils/OpSet.h>

class Mesh;
class Chain;
class Metadata;
class AtomGroup;

namespace MabUtils
{
	template <class Object>
	std::string gather_validations
	(const std::function<std::string(Object &)> &validate,
	 std::list<Object> &objects)
	{
		std::string problems;
		for (Object &object : objects)
		{
			std::string contribution = validate(object);
			if (contribution.length())
			{
				problems += contribution;
				problems += "\n";
			}
		}
		if (problems.length() > 0)
		{
			problems.pop_back();
		}
		if (objects.size() == 0)
		{
			problems += "Nothing defined";
		}
		return problems;
	};
};

struct Antigen
{
	std::string title{};
	Model model{};
	OpSet<std::string> entities;
	Mesh *_mesh{};
	
	std::vector<Instance *> instances();
	
	std::string validate() const;
	
	Mesh *mesh();

	void wipe() { /* fill me in */ }
};

class Antigens : public std::list<Antigen>
{
public:
	std::string validate();

	OpSet<std::string> entities() const;
	
	const Antigen *antigen(const std::string &id) const;
	Antigen *antigen(const std::string &id);
};

struct Competition
{
	std::string filename{};
	Metadata *metadata{};

	std::string left_header;
	std::string right_header;
	std::string value_header;
	
	bool as_competition{false};
	float scale{1};
	std::string interpretation_as_desc();
	
	std::string antigen;

	std::string validate(const Antigens &antigens) const;
};

class Competitions : public std::list<Competition>
{
public:
	std::string validate(const Antigens &antigens);

	OpSet<std::string> all_antibodies();
};

struct Fiducial
{
	std::string name;
	Model model{};
	OpSet<std::string> entities;
	OpSet<std::string> non_antigen_entities(const Antigens &antigens);

	std::string antigen;

	std::string validate(const Antigens &antigens);
};

class Fiducials : public std::list<Fiducial>
{
public:
	OpSet<std::string> all_fiducials();

	std::string validate(const Antigens &antigens);
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
	Competitions competitions{};
	Fiducials fiducials{};
	ColourMap colours{};
};

#endif
