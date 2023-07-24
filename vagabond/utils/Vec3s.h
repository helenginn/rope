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

#ifndef __vagabond__Vec3s__
#define __vagabond__Vec3s__

#include "glm_json.h"
#include "OpVec.h"

typedef OpVec<glm::vec3> Vec3s;
typedef OpVec<float> Floats;

#include "Point.h"
#include "Mapping.h"

inline void to_json(json &j, const Point<2, Vec3s> &point)
{
	std::vector<float> coords = point;
	Vec3s value = point.value();

	j["coords"] = coords;
	j["val"] = value;
}

inline void from_json(const json &j, Point<2, Vec3s> &point)
{
	if (j.count("coords"))
	{
		std::vector<float> coords = j.at("coords");
		point.set_vector(coords);
	}

	if (j.count("val"))
	{
		const Vec3s &vs = j.at("val");
		point.set_value(vs);
	}
}

inline void to_json(json &j, const Mapping<2, Vec3s> &map)
{
	typedef SharedFace<0, 2, Vec3s> HyperPoint;
	std::vector<HyperPoint> points;
	for (auto p : map._points)
	{
		points.push_back(*p);
	}
	j["points"] = points;

	for (int i = 0; i < map._mapped.size(); i++)
	{
		std::vector<int> tIndices;
		map.point_indices_for_face(i, tIndices);
		j["face"][i] = tIndices;
	}

}

inline void from_json(const json &j, Mapping<2, Vec3s> &map)
{
	typedef SharedFace<0, 2, Vec3s> HyperPoint;
	if (j.count("points"))
	{
		for (HyperPoint p : j.at("points"))
		{
			HyperPoint *hp = new HyperPoint(p);
			map.add_point(hp);
		}
	}
	
	if (j.count("face"))
	{
		for (std::vector<int> idxs : j.at("face"))
		{
			std::vector<HyperPoint *> points;
			
			for (const int &idx : idxs)
			{
				points.push_back(map.point(idx));
			}

			map.add_simplex(points);
		}
	}
}

///////////////////////////////////////////////////////////

inline void to_json(json &j, const Point<2, Floats> &point)
{
	std::vector<float> coords = point;
	Floats value = point.value();

	j["coords"] = coords;
	j["val"] = value;
	
	j["grad"] = point._grads;
}

inline void from_json(const json &j, Point<2, Floats> &point)
{
	if (j.count("coords"))
	{
		std::vector<float> coords = j.at("coords");
		point.set_vector(coords);
	}

	if (j.count("val"))
	{
		const Floats &vs = j.at("val");
		point.set_value(vs);

		Floats grads; grads.resize(vs.size());
		for (size_t i = 0; i < 2; i++)
		{
			point._grads[i] = grads;
		}
	}

	if (j.count("grad"))
	{
		std::vector<Floats> grads = j.at("grad");
		for (size_t i = 0; i < 2; i++)
		{
			point._grads[i] = grads[i];
		}
	}
}

inline void to_json(json &j, const Mapping<2, Floats> &map)
{
	typedef SharedFace<0, 2, Floats> HyperPoint;
	std::vector<HyperPoint> points;
	for (auto p : map._points)
	{
		points.push_back(*p);
	}
	j["points"] = points;

	for (int i = 0; i < map._mapped.size(); i++)
	{
		std::vector<int> tIndices;
		map.point_indices_for_face(i, tIndices);
		j["face"][i] = tIndices;
	}

}

inline void from_json(const json &j, Mapping<2, Floats> &map)
{
	typedef SharedFace<0, 2, Floats> HyperPoint;
	if (j.count("points"))
	{
		for (HyperPoint p : j.at("points"))
		{
			HyperPoint *hp = new HyperPoint(p);
			map.add_point(hp);
		}
	}
	
	if (j.count("face"))
	{
		for (std::vector<int> idxs : j.at("face"))
		{
			std::vector<HyperPoint *> points;
			
			for (const int &idx : idxs)
			{
				points.push_back(map.point(idx));
			}

			map.add_simplex(points);
		}
	}
}


#endif
