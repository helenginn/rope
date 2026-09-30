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

#ifndef __vagabond__RefinedResult__
#define __vagabond__RefinedResult__

#include <vagabond/core/Model.h>
#include "Positions.h"

// the outcome of one completed refinement from random starting positions
struct RefinedResult
{
	std::vector<Positions::Placement> placements;
	float score{};
};

inline void to_json(json &j, const Positions::Placement &p)
{
	j["name"] = p.name;
	j["fixed"] = p.fixed;
	j["meshIdx"] = p.meshIdx;

	json copies = json::array();
	for (const glm::vec3 &v : p.copies)
	{
		copies.push_back({v.x, v.y, v.z});
	}
	j["copies"] = copies;
}

inline void from_json(const json &j, Positions::Placement &p)
{
	p.name = j.at("name");
	p.fixed = j.at("fixed");
	p.meshIdx = j.at("meshIdx");

	p.copies.clear();
	for (const json &v : j.at("copies"))
	{
		p.copies.push_back({v[0], v[1], v[2]});
	}
}

inline void to_json(json &j, const RefinedResult &r)
{
	j["placements"] = r.placements;
	j["score"] = r.score;
}

inline void from_json(const json &j, RefinedResult &r)
{
	r.placements = j.at("placements").get<std::vector<Positions::Placement>>();
	r.score = j.at("score");
}

// a named selection of results (e.g. from "Pick best") kept with the bucket of
// fixed antibodies they were made with (see ResultsView::bucketKey())
struct ResultGroup
{
	std::string key;
	std::string name;
	std::vector<RefinedResult> results;
};

inline void to_json(json &j, const ResultGroup &g)
{
	j["key"] = g.key;
	j["name"] = g.name;
	j["results"] = g.results;
}

inline void from_json(const json &j, ResultGroup &g)
{
	g.key = j.at("key");
	g.name = j.at("name");
	g.results = j.at("results").get<std::vector<RefinedResult>>();
}

#endif
