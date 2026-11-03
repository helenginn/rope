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
#include "Symmetry.h"
#include <vagabond/core/Model.h>
#include "RefinedResult.h"
#include <vagabond/utils/OpSet.h>
#include <vagabond/utils/Eigen/Dense>

class Mesh;
class Chain;
class Metadata;
class AtomGroup;
class ContactPoint;

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
	Symmetry sym{};

	// positions of the mesh vertices excluded from where refinements start
	// (positions, not indices, so that they survive the mesh being made
	// again); see Mesh::excludeSelection()
	std::vector<glm::vec3> excluded_start;

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

	json excluded = json::array();
	for (const glm::vec3 &p : a.excluded_start)
	{
		excluded.push_back({p.x, p.y, p.z});
	}
	j["excluded_start"] = excluded;
}

inline void from_json(const json &j, Antigen &a)
{
	a.title = j.at("title");
	a.model = j.at("model");
	a.entities = j.at("entities");

	if (j.count("excluded_start"))
	{
		for (const json &p : j.at("excluded_start"))
		{
			a.excluded_start.push_back(glm::vec3(p.at(0).get<float>(),
			                                     p.at(1).get<float>(),
			                                     p.at(2).get<float>()));
		}
	}
}

class Antigens : public std::list<Antigen>
{
public:
	std::string validate();

	OpSet<std::string> entities() const;
	
	const Antigen *antigen(const std::string &id) const;
	Antigen *antigen(const std::string &id);

	Antigen *operator()(const std::string &id)
	{
		return antigen(id);
	}
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
	OpSet<std::string> antibody_names() const;
	
	std::vector<std::string> favoured_ordering;

	// antibodies put aside: left out of the matrix, the fit and the
	// refinement (see clean_order()), but kept until they are put back
	std::vector<std::string> naughty_corner;

	// which antibodies with structural information were toggled to be fixed
	// at their structural position, last time (see Positions::setFixed()).
	// Only meaningful once saved: until then every one of them is fixed.
	std::vector<std::string> fixed_antibodies;
	bool fixed_saved{false};

	// how big an antibody is, in Angstroms, for how close two have to be to
	// compete (see PosToComp::closeness())
	float radius{11.f};

	// How much an antibody's radius changes with how curved the mesh is under
	// it: Angstroms per standard deviation of curvature (see
	// Mesh::curvatureZ()), more on convex ground and less on concave. 0 is off.
	float curvature_scale{2.f};

	// Whether a fixed antibody with a footprint on the mesh (see
	// Positions::buildFootprints()) takes the place of its blanket radius:
	// another antibody competes with it by how near the footprint it is.
	bool use_footprints{true};

	// cached refinements (see PosToComp), saved to and loaded from json
	std::vector<RefinedResult> results;
	// named selections of those results, such as the best picks
	std::vector<ResultGroup> groups;

	std::string validate(const Antigens &antigens) const;

	/** Competition worked out from the rest of the data: for two antibodies
	 *  with no entry of their own, a third which competes strongly with both
	 *  (see infer()). Kept apart from the data, which is never changed; while
	 *  useInferred is on they fill in the gaps of make_plot(), and count
	 *  towards the fit in proportion to their value. */
	struct Inferred
	{
		std::string left;
		std::string right;
		float value{};
	};

	std::vector<Inferred> inferred;
	bool useInferred{false};

	/** A single Floyd-Warshall step over the data: wherever i competes with
	 *  j and j with k, both above 0.5, i competes with k by the product of the
	 *  two (each counted as 1 at most), if there is nothing of its own for
	 *  that pair. The strongest route wins. Only the data itself is used, never
	 *  a value inferred from it, so nothing is inferred from a chain of more
	 *  than two. Returns how many pairs were inferred; any earlier inference
	 *  is replaced. */
	size_t infer();
	// NaN if the pair was not inferred
	float inferredValue(const std::string &ab1, const std::string &ab2) const;
	/** how much each entry of make_plot() counts for with this order (as
	 *  it leaves it): 1 for the data, the value for one inferred, else 0 */
	Eigen::MatrixXf make_weights(const std::vector<std::string> &order);

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

	j["favoured_ordering"] = c.favoured_ordering;
	j["naughty_corner"] = c.naughty_corner;
	if (c.fixed_saved)
	{
		j["fixed_antibodies"] = c.fixed_antibodies;
	}
	j["radius"] = c.radius;
	j["curvature_scale"] = c.curvature_scale;
	j["use_footprints"] = c.use_footprints;
	j["as_competition"] = c.as_competition;
	j["scale"] = c.scale;
	j["antigen"] = c.antigen;
	j["results"] = c.results;
	j["groups"] = c.groups;

	json inferred = json::array();
	for (const Competition::Inferred &i : c.inferred)
	{
		inferred.push_back({{"left", i.left}, {"right", i.right}, 
		                    {"value", i.value}});
	}
	j["inferred"] = inferred;
	j["use_inferred"] = c.useInferred;
}

inline void from_json(const json &j, Competition &c)
{
	c.filename = j.at("filename");
	c.metadata = new Metadata(j.at("metadata"));

	c.left_header = j.at("left_header");
	c.right_header = j.at("right_header");
	c.value_header = j.at("value_header");

	if (j.count("favoured_ordering"))
	{
		c.favoured_ordering = j.at("favoured_ordering");
	}

	if (j.count("radius"))
	{
		c.radius = j.at("radius");
	}

	if (j.count("use_footprints"))
	{
		c.use_footprints = j.at("use_footprints");
	}

	if (j.count("curvature_scale"))
	{
		c.curvature_scale = j.at("curvature_scale");
	}

	if (j.count("naughty_corner"))
	{
		c.naughty_corner = j.at("naughty_corner").get<std::vector<std::string>>();
	}

	if (j.count("fixed_antibodies"))
	{
		c.fixed_antibodies = j.at("fixed_antibodies").get<std::vector<std::string>>();
		c.fixed_saved = true;
	}

	c.as_competition = j.at("as_competition");
	c.scale = j.at("scale");
	c.antigen = j.at("antigen");

	if (j.count("results"))
	{
		c.results = j.at("results").get<std::vector<RefinedResult>>();
	}

	if (j.count("groups"))
	{
		c.groups = j.at("groups").get<std::vector<ResultGroup>>();
	}

	if (j.count("inferred"))
	{
		for (const json &i : j.at("inferred"))
		{
			c.inferred.push_back({i.at("left"), i.at("right"), i.at("value")});
		}
	}

	if (j.count("use_inferred"))
	{
		c.useInferred = j.at("use_inferred");
	}
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
	ContactPoint *contact{};
	// the saved result of aligning this onto its antigen (see ContactPoint),
	// so that it need not be worked out again until its inputs change
	json alignment{};

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

	if (!f.alignment.is_null())
	{
		j["alignment"] = f.alignment;
	}
}

inline void from_json(const json &j, Fiducial &f)
{
	f.name = j.at("name");
	f.model = j.at("model");
	f.entities = j.at("entities");
	f.antigen = j.at("antigen");

	if (j.count("alignment"))
	{
		f.alignment = j.at("alignment");
	}
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
