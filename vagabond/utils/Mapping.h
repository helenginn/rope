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
#include <set>
#include <float.h>

template <typename Type>
class Mapped
{
public:
	virtual ~Mapped()
	{

	}
	virtual void bounds(std::vector<float> &min, std::vector<float> &max) = 0;
	virtual Type interpolate_variable(const std::vector<float> &cart) = 0;

	virtual void fraction_to_real(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max) = 0;

	virtual bool acceptable_coordinate(const std::vector<float> &cart) = 0;
};

template <unsigned int D, typename Type>
class Mapping : public Mapped<Type>
{
public:
	virtual ~Mapping()
	{

	}
	Face<2, D, Type> *addTriangle(Face<0, D, Type> *p, Face<0, D, Type> *q,
	                 Face<0, D, Type> *r)
	{
		Face<1, D, Type> *pq = new Face<1, D, Type>(*p, *q);
		Face<2, D, Type> *pqr = new Face<2, D, Type>(*pq, *r);

		addFace(pqr);
		return pqr;
	}

	template <class Face>
	void addFace(Face *face)
	{
		if (std::find(_mapped.begin(), _mapped.end(), face) !=
		    _mapped.end())
		{
			return;
		}

		for (size_t i = 0; i < face->pointCount(); i++)
		{
			addPoint(&face->v_point(i));
		}
		
		_mapped.push_back(face);
	}

	virtual bool acceptable_coordinate(const std::vector<float> &cart)
	{
		return (face_for_point(cart) != nullptr);
	}
	
	Type interpolate_variable(const std::vector<float> &cart)
	{
		Mappable<Type> *f = face_for_point(cart);
		if (f == nullptr)
		{
			return Type{};
		}
		
		Type t = f->interpolate_subfaces(cart);
		return t;
	}
	
	SharedFace<0, D, Type> &point(int idx)
	{
		return _points[idx];
	}

	size_t pointCount() const
	{
		return _points.size();
	}
	
	virtual void bounds(std::vector<float> &min, std::vector<float> &max)
	{
		min = std::vector<float>(D, FLT_MAX);
		max = std::vector<float>(D, -FLT_MAX);
		for (SharedFace<0, D, Type> *point : _points)
		{
			std::vector<float> p = *point;
			for (int i = 0; i < p.size(); i++)
			{
				if (min[i] > p[i]) { min[i] = p[i]; }
				if (max[i] < p[i]) { max[i] = p[i]; }
			}
		}
	}
	
	virtual void fraction_to_real(std::vector<float> &val,
	                              const std::vector<float> &min, 
	                              const std::vector<float> &max)
	{
		assert(min.size() == D);
		assert(max.size() == D);
		std::vector<float> res(D);
		for (int i = 0; i < D; i++)
		{
			float old = val[i];
			val[i] = old * (max.at(i) - min.at(i)) + min.at(i);
		}
	}
protected:
	Mappable<Type> *face_for_point(const std::vector<float> &point)
	{
		for (Mappable<Type> *face : _mapped)
		{
			std::vector<float> bcp = face->point_to_barycentric(point);
			bool positive = true;
			for (float &f : bcp)
			{
				if (f < 0)
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
	void addPoint(SharedFace<0, D, Type> *point)
	{
		if (std::find(_points.begin(), _points.end(), point) !=
		    _points.end())
		{
			return;
		}

		_points.push_back(point);
	}
private:
	std::vector<SharedFace<0, D, Type> *> _points;
	std::vector<Mappable<Type> *> _mapped;

};

#endif
