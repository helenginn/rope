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

#ifndef __vagabond__Mapping__
#define __vagabond__Mapping__

#include "Face.h"
#include <map>
#include <float.h>

#include <nlohmann/json.hpp>
using nlohmann::json;


template <typename Type>
class Mapped
{
public:
	virtual ~Mapped()
	{

	}
	virtual void bounds(std::vector<float> &min, std::vector<float> &max) = 0;
	virtual Type interpolate_variable(const std::vector<float> &cart,
	                                  bool *acceptable = nullptr) const = 0;

	virtual void real_to_fraction(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max) = 0;

	virtual void fraction_to_real(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max) = 0;

	virtual bool acceptable_coordinate(const std::vector<float> &cart) = 0;
	virtual void crack_existing_face(int idx) = 0;
	virtual std::vector<float> middle_of_face(int tidx) = 0;
	virtual void face_indices_for_point(const int &pidx, 
	std::vector<int> &idxs) = 0;
	virtual bool face_has_point(int idx, const std::vector<float> &point) = 0;
	virtual int face_idx_for_point(const std::vector<float> &point) = 0;
	virtual void point_indices_for_face(int idx, std::vector<int> &points) const = 0;
	virtual void update(int point_idx) = 0;
	virtual void redo_bins() = 0;
	
	virtual int n() = 0;

	virtual std::vector<Mappable<Type> *> simplicesForPointDim(int index, 
	                                                           int dimension) = 0;
	virtual Mappable<Type> * simplex_for_points(std::vector<int> &points) = 0;
	virtual Mappable<Type> * face_for_index(int i) = 0;

	void update()
	{
		update(-1);
	}
	
	// returns index of added point
	virtual int add_point(const std::vector<float> &cart) = 0;
	virtual std::vector<float> point_vector(int i) = 0;
	virtual void set_point_vector(int i, const float *coords) = 0;
	virtual size_t pointCount() const = 0;
	virtual size_t faceCount() const = 0;
	virtual void remove_point(int idx) = 0;
	virtual void alter_value(int idx, Type value) = 0;
	virtual Type &get_value(int idx) = 0;
	virtual void delaunay_refine() = 0;
};

template <unsigned int D, typename Type>
class Mapping : public Mapped<Type>
{
public:
	typedef SharedFace<D, D, Type> HyperTriangle;
	typedef SharedFace<D - 1, D, Type> HyperLine;
	typedef SharedFace<0, D, Type> HyperPoint;
	typedef std::set<MappedInDim<D, Type> *> SimplexVector;
	
	Mapping()
	{

	}

	virtual int n()
	{
		return D;
	}

	template <class Other>
	Mapping(Mapping<D, Other> &blueprint)
	{
		typedef SharedFace<0, D, Other> OtherPoint;
		typedef SharedFace<D, D, Other> OtherTriangle;

		std::map<OtherPoint *, HyperPoint *> translation;
		for (int i = 0; i < blueprint.pointCount(); i++)
		{
			OtherPoint *op = blueprint.point(i);
			HyperPoint *myPoint = new HyperPoint(*op);
			add_point(myPoint);
			translation[op] = myPoint;
		}
		
		for (int i = 0; i < blueprint.faceCount(); i++)
		{
			OtherTriangle *of = blueprint.face(i);
			std::vector<HyperPoint *> points;

			for (size_t j = 0; j < of->pointCount(); j++)
			{
				OtherPoint *op = &of->v_point(j);
				HyperPoint *hp = translation[op];
				points.push_back(hp);
			}
			
			add_simplex(points);
		}
	}

	virtual ~Mapping()
	{

	}

	template <int N>
	Face<N + 1, D, Type> *next_simplex(SharedFace<N, D, Type> *last, 
	                                        HyperPoint *next)
	{
		Face<N + 1, D, Type> *combine;
		combine = new Face<N + 1, D, Type>(*last, *next);
		return combine;
	}
	
	virtual void redo_bins()
	{
		_simplices.clear();
		for (HyperTriangle *triangle : _mapped)
		{
			triangle->add_to_bins(_simplices);
		}
	}
	
	HyperTriangle *add_simplex(const std::vector<HyperPoint *> &ps)
	{
		SharedFace<D, D, Type> *last;
		last = SharedFace<D, D, Type>::make_next(ps);
		last->add_to_bins(_simplices);
		
		HyperTriangle *tr = static_cast<HyperTriangle *>(last);
		add_face(tr);
		return tr;
	}

	void add_face(HyperTriangle *face)
	{
		for (size_t i = 0; i < face->pointCount(); i++)
		{
			HyperPoint *hp = &face->v_point(i);
			add_point(hp);
			_members[hp].push_back(face);
		}
		
		if (std::find(_mapped.begin(), _mapped.end(), face) !=
		    _mapped.end())
		{
			return;
		}

		_mapped.push_back(face);
	}
	
	HyperTriangle *face(int idx)
	{
		return _mapped[idx];
	}
	
	void remove_face(HyperTriangle *face)
	{
		for (int i = 0; i < face->pointCount(); i++)
		{
			HyperPoint *hp = &face->v_point(i);
			if (_members.count(hp))
			{
				std::vector<HyperTriangle *> &hts = _members[hp];
				
				auto it = std::find(hts.begin(), hts.end(), face);
				
				if (it != hts.end())
				{
					hts.erase(it);
				}
			}
		}

		auto it = std::find(_mapped.begin(), _mapped.end(), face);
		
		if (it != _mapped.end())
		{
			_mapped.erase(it);
		}
	}
	
	virtual void remove_point(int idx)
	{
		HyperPoint *p = point(idx);
		bool found = true;
		while (found)
		{
			found = false;
			for (HyperTriangle *m : _mapped)
			{
				if (m->hasPoint(*p))
				{
					found = true;
					remove_face(m);
					break;
				}
			}
		}
		
		_points.erase(_points.begin() + idx);
	}

	virtual void crack_existing_face(int idx)
	{
		HyperPoint *point = _points[idx];
		std::vector<float> pos = *point;
		HyperTriangle *face = face_for_point(pos);
		if (!face)
		{
			std::cout << "no face found to crack" << std::endl;
			return;
		}

		remove_face(face);
		
		for (int i = 0; i < face->pointCount(); i++)
		{
			HyperLine *sf = face->faceExcluding(face->point(i));
			SharedFace<D - 1, D, Type> *f;
			f = static_cast<SharedFace<1, D, Type> *>(sf);
			HyperTriangle *replace = new Face<2, D, Type>(*f, *point);
			add_face(replace);
		}
	}

	virtual void set_point_vector(int i, const float *coords)
	{
		_points[i]->set_vector(coords);
	}

	virtual std::vector<float> point_vector(int i)
	{
		return *_points[i];
	}

	virtual bool acceptable_coordinate(const std::vector<float> &cart)
	{
		return (face_for_point(cart) != nullptr);
	}
	
	Type interpolate_variable(const std::vector<float> &cart, 
	                          bool *acceptable = nullptr) const
	{
		MappedInDim<D, Type> *f = face_for_point(cart);
		if (f == nullptr)
		{
			if (acceptable != nullptr)
			{
				*acceptable = false;
			}
			return Type{};
		}
		
		Type t = f->interpolate_subfaces(cart);
		if (acceptable != nullptr)
		{
			*acceptable = true;
		}
		return t;
	}
	
	HyperPoint *const &point(int idx) const
	{
		return _points[idx];
	}

	HyperPoint *point(int idx)
	{
		return _points[idx];
	}

	virtual size_t pointCount() const
	{
		return _points.size();
	}

	virtual size_t faceCount() const
	{
		return _mapped.size();
	}
	
	SimplexVector &bin(int i)
	{
		return _simplices[i];
	}
	
	virtual void bounds(std::vector<float> &min, std::vector<float> &max)
	{
		min = std::vector<float>(D, FLT_MAX);
		max = std::vector<float>(D, -FLT_MAX);
		for (HyperPoint *point : _points)
		{
			std::vector<float> p = *point;
			for (int i = 0; i < p.size(); i++)
			{
				if (min[i] > p[i]) { min[i] = p[i]; }
				if (max[i] < p[i]) { max[i] = p[i]; }
			}
		}
	}
	
	virtual void real_to_fraction(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max)
	{
		std::vector<float> res(D);
		for (int i = 0; i < D; i++)
		{
			float old = val[i];
			val[i] = (old - min.at(i)) / (max.at(i) - min.at(i));
		}
	}

	virtual void fraction_to_real(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max)
	{
		std::vector<float> res(D);
		for (int i = 0; i < D; i++)
		{
			float old = val[i];
			val[i] = old * (max.at(i) - min.at(i)) + min.at(i);
		}
	}

	virtual Type &get_value(int idx)
	{
		return _points[idx]->v_value();
	}

	virtual void alter_value(int idx, Type value)
	{
		_points[idx]->set_value(value);
	}

	virtual int add_point(const std::vector<float> &cart)
	{
		HyperPoint *point = new HyperPoint(cart);
		if (add_point(point))
		{
			return pointCount() - 1;
		}
		return -1;
	}

	virtual bool add_point(HyperPoint *point)
	{
		if (std::find(_points.begin(), _points.end(), point) !=
		    _points.end())
		{
			return false;
		}

		_points.push_back(point);
		return true;
	}
	
	int points_in_circumcentre(HyperTriangle *triangle)
	{
		float rad = 0; int count = 0;
		std::vector<float> cc = triangle->cartesian_circumcenter(&rad);

		for (int i = 0; i < pointCount(); i++)
		{
			if (!_members.count(point(i)) || _members.at(point(i)).size() == 0)
			{
				continue;
			}
			
			if (triangle->hasPoint(*point(i)))
			{
				continue;
			}

			bool included = point(i)->is_within_hypersphere(cc, rad);
			
			count += (included ? 1 : 0);
		}
		
		return count;
	}
	
	std::vector<HyperTriangle *> find_other_triangles(HyperTriangle *other)
	{
		std::vector<HyperTriangle *> neighbours;

		for (int i = 0; i < other->pointCount(); i++)
		{
			HyperPoint *hp = &other->v_point(i);
			std::vector<HyperTriangle *> &list = _members[hp];

			for (HyperTriangle *tri : list)
			{
				if (tri == other) { continue; }
				
				if (tri->shared_point_count(other) != D)
				{
					continue;
				}

				neighbours.push_back(tri);
			}
		}

		return neighbours;
	}
	
	int total_delaunay_score(std::vector<HyperTriangle *> &nexts)
	{
		int sum = 0;
		nexts.clear();

		for (HyperTriangle *triangle : _mapped)
		{
			int count = points_in_circumcentre(triangle);
			
			if (count > 0)
			{
				nexts.push_back(triangle);
			}

			sum += count;
		}

		return sum;
	}
	
	bool try_swap(HyperTriangle *next, HyperTriangle *pair)
	{
		std::set<HyperPoint *> shared = next->shared_points(pair);

		int reference = 0;
		reference += points_in_circumcentre(next);
		reference += points_in_circumcentre(pair);

		HyperPoint *p = next->point_not_in(pair);
		HyperPoint *q = pair->point_not_in(next);

		for (HyperPoint *hp : shared)
		{
			for (HyperPoint *hq : shared)
			{
				if (hp == hq)
				{
					continue;
				}

				HyperTriangle *renext = new HyperTriangle(*next);
				HyperTriangle *repair = new HyperTriangle(*pair);

				renext->swap_point(hp, q);
				repair->swap_point(hq, p);

				int trial = 0;
				trial += points_in_circumcentre(renext);
				trial += points_in_circumcentre(repair);

				if (trial < reference)
				{
					remove_face(next); remove_face(pair);
					add_face(renext); add_face(repair);

					delete next; delete pair;
					return true;
				}
				else
				{
					delete renext; delete repair;
				}
			}
		}
		
		return false;
	}

	virtual bool delaunay_refine(HyperTriangle *next)
	{
		std::vector<HyperTriangle *> nbs = find_other_triangles(next);
		if (nbs.size() == 0)
		{
			return false;
		}

		for (HyperTriangle *neighbour : nbs)
		{
			if (try_swap(next, neighbour))
			{
				return true;
			}
		}
		
		return false;
	}
	
	virtual void delaunay_refine()
	{
		std::vector<HyperTriangle *> nexts;
		int sum = total_delaunay_score(nexts);
		
		while (sum > 0)
		{
			bool success = false;
			for (HyperTriangle *next : nexts)
			{
				success = delaunay_refine(next);
				
				if (success)
				{
					break;
				}
			}
			
			if (!success)
			{
				break;
			}

			sum = total_delaunay_score(nexts);
		}
	}
	
	bool points_share_triangle(HyperPoint *p, HyperPoint *q)
	{
		for (HyperTriangle *tp : _members[p])
		{
			for (HyperTriangle *tq : _members[q])
			{
				if (tp == tq) { return true; }
			}
		}

		return false;
	}

	virtual int face_idx_for_point(const std::vector<float> &point)
	{
		for (size_t i = 0; i < _mapped.size(); i++)
		{
			if (face_has_point(i, point))
			{
				return i;
			}
		}

		return -1;
	}
	
	virtual bool face_has_point(int idx, const std::vector<float> &point)
	{
		HyperTriangle *face = _mapped[idx];

		if (!face->point_in_bounds(point))
		{
			return false;
		}

		const std::vector<float> &bcp = face->point_to_barycentric(point);
		for (const float &f : bcp)
		{
			if (f < -1e-6)
			{
				return false;
			}
		}

		return true;
	}

	virtual void point_indices_for_face(int idx, std::vector<int> &points) const
	{
		for (int i = 0; i < pointCount(); i++)
		{
			if (_mapped[idx]->hasPoint(*point(i)))
			{
				points.push_back(i);
			}
		}
	}

	virtual void face_indices_for_point(const int &pidx, std::vector<int> &idxs)
	{
		for (int i = 0; i < _mapped.size(); i++)
		{
			if (face_has_point(i, *point(pidx)))
			{
				idxs.push_back(i);
			}
		}
	}

	void update()
	{
		update(-1);
	}

	virtual void update(int idx)
	{
		if (idx >= 0)
		{
			for (HyperTriangle *ht : _members[_points[idx]])
			{
				ht->changed();
			}
			return;
		}

		for (int i = 0; i < _mapped.size(); i++)
		{
			_mapped[i]->changed();
		}
	}
	
	virtual Mappable<Type> * simplex_for_points(std::vector<int> &points)
	{
		int dimension = points.size() - 1;
		for (MappedInDim<D, Type> *simplex : bin(dimension))
		{
			bool found = true;
			for (const int &idx : points)
			{
				if (!simplex->hasPoint(*_points[idx]))
				{
					found = false;
					continue;
				}
			}
			
			if (found)
			{
				return simplex;
			}
		}

		return nullptr;
	}

	virtual std::vector<float> middle_of_face(int tidx)
	{
		float pc = pointCount();
		std::vector<float> bc(pc, 1 / pc);
		return _mapped[tidx]->barycentric_to_point(bc);
	}

	virtual Mappable<Type> *face_for_index(int i)
	{
		return _mapped[i];
	}
	
	std::vector<Mappable<Type> *> simplicesForPointDim(int index, int dimension)
	{
		std::vector<Mappable<Type> *> ret;
		
		HyperPoint *needle = _points[index];
		for (MappedInDim<D, Type> *simplex : bin(dimension))
		{
			if (simplex->hasPoint(*needle))
			{
				ret.push_back(simplex);
			}
		}

		return ret;
	}

	friend void to_json(json &j, const Mapping<D, Type> &face);
	friend void from_json(const json &j, Mapping<D, Type> &face);
protected:
	HyperTriangle *face_for_point(const std::vector<float> &point) const
	{
		for (HyperTriangle *face : _mapped)
		{
			if (!face->point_in_bounds(point))
			{
				continue;
			}

			const std::vector<float> &bcp = face->point_to_barycentric(point);
			bool positive = true;
			for (const float &f : bcp)
			{
				if (f < -1e-6)
				{
					positive = false;
				}
			}
			
			if (positive)
			{
				return face;
			}
		}

		return nullptr;
	}
private:
	std::vector<HyperPoint *> _points;
	std::vector<HyperTriangle *> _mapped;

	std::map<HyperPoint *, std::vector<HyperTriangle *>> _members;
	std::map<int, SimplexVector> _simplices;
};

inline void to_json(json &j, const Mapping<2, float> &map)
{
	typedef SharedFace<0, 2, float> HyperPoint;
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

inline void from_json(const json &j, Mapping<2, float> &map)
{
	typedef SharedFace<0, 2, float> HyperPoint;
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
	
	map.update();
	map.redo_bins();
}

#endif
