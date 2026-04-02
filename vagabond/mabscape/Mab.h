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
#include <vagabond/utils/Eigen/Dense>

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

	friend void to_json(json &j, const Antigen &a);
	friend void from_json(const json &j, Antigen &a);
	void housekeeping();
};

inline void to_json(json &j, const Antigen &a)
{
	j["title"] = a.title;
	j["model"] = a.model;
	j["entities"] = a.entities;
}

inline void from_json(const json &j, Antigen &a)
{
	a.title = j.at("title");
	a.model = j.at("model");
	a.entities = j.at("entities");
}

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
	TabulatedData *_asData{};

	TabulatedData *asData();

	Eigen::MatrixXf make_plot(std::vector<std::string> &order);

	std::vector<std::string> 
	clean_order(const std::vector<std::string> &order);

	std::string left_header;
	std::string right_header;
	std::string value_header;
	
	bool as_competition{false};
	float scale{1};
	
	float value(const std::string &left, const std::string &right);
	void adjust_value(float &val);
	
	std::string antigen;

	std::string interpretation_as_desc();
	OpSet<std::string> antibody_names();
	
	std::vector<std::string> favoured_ordering;

	std::string validate(const Antigens &antigens) const;

	friend void to_json(json &j, const Competition &c);
	friend void from_json(const json &j, Competition &c);
};

inline void to_json(json &j, const Competition &c)
{
	j["filename"] = c.filename;
	j["metadata"] = *c.metadata;

	j["left_header"] = c.left_header;
	j["right_header"] = c.right_header;
	j["value_header"] = c.value_header;

	j["as_competition"] = c.as_competition;
	j["scale"] = c.scale;
	j["antigen"] = c.antigen;
}

inline void from_json(const json &j, Competition &c)
{
	c.filename = j.at("filename");
	c.metadata = new Metadata(j.at("metadata"));;

	c.left_header = j.at("left_header");
	c.right_header = j.at("right_header");
	c.value_header = j.at("value_header");

	c.as_competition = j.at("as_competition");
	c.scale = j.at("scale");
	c.antigen = j.at("antigen");
}

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
	void housekeeping();

	friend void to_json(json &j, const Fiducial &f);
	friend void from_json(const json &j, Fiducial &f);
};

inline void to_json(json &j, const Fiducial &f)
{
	j["name"] = f.name;
	j["model"] = f.model;
	j["entities"] = f.entities;
	j["antigen"] = f.antigen;
}

inline void from_json(const json &j, Fiducial &f)
{
	f.name = j.at("name");
	f.model = j.at("model");
	f.entities = j.at("entities");
	f.antigen = j.at("antigen");
}

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
	
	void save();
	void load();
	void housekeeping();

	friend void to_json(json &j, const Mab &m);
	friend void from_json(const json &j, Mab &m);
};

inline void to_json(json &j, const Mab &m)
{
	j["antigens"] = m.antigens;
	j["competitions"] = m.competitions;
	j["fiducials"] = m.fiducials;
}

inline void from_json(const json &j, Mab &m)
{
	m.antigens = j.at("antigens");
	if (j.count("competitions"))
	{
		m.competitions = j.at("competitions");
	}
	if (j.count("fiducials"))
	{
		m.fiducials = j.at("fiducials");
	}
}

#endif
