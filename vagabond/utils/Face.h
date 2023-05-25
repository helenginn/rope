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

#ifndef __vagabond__Face__
#define __vagabond__Face__

#include <iostream>
#include "svd/PCA.h"
#include "Variable.h"

template <unsigned int D, typename Type>
class Point
{
public:
	Point() {}
	Point(const float *p, Type value = 0)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
		
		_value = value;
	}
	
	operator std::vector<float>()
	{
		std::vector<float> res(D);
		for (int i = 0; i < D; i++)
		{
			res[i] = _p[i];
		}
		return res;
	}
	
	Point(const std::vector<float> &p, Type value = 0)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
		
		_value = value;
	}
	
	const float &scalar(int idx) const
	{
		return _p[idx];
	}
	
	float &operator[](int idx)
	{
		return _p[idx];
	}
	
	float *p()
	{
		return _p;
	}
	
	const Type &value() const
	{
		return _value;
	}
private:
	float _p[D]{};
	Type _value{};

	unsigned int _d = D;

	friend std::ostream &operator<<(std::ostream &ss, const Point &p)
	{
		ss << "(";
		for (size_t i = 0; i < p._d; i++)
		{
			ss << p._p[i];
			if (i < p._d - 1)
			{
				ss << ", ";
			}
		}
		ss << ") = " << p._value << std::endl;
		return ss;
	}
};

template <class Type>
class Mappable
{
public:
	virtual Type interpolate_subfaces(const std::vector<float> &cart) = 0;
	virtual std::vector<float> point_to_barycentric(const std::vector<float> &m) = 0;

	virtual ~Mappable()
	{

	}

};

template <int N, unsigned int D, typename Type>
class SharedFace;

template <int N, unsigned int D, typename Type>
class Interpolatable : public Mappable<Type>
{
public:
	Interpolatable()
	{
		setupMatrix(&_vec, D + 1, 1);
		setupSVD(&_svd, D+1, 0);

	}
	
	virtual ~Interpolatable()
	{

	}
	
	void setVariable(Variable<Type> *var)
	{
		_variable = var;
	}

	virtual SharedFace<N-1, D, Type> *faceExcluding(const 
	                                                SharedFace<0, D, Type> *point)
	{
		return nullptr;
	}

	virtual Type interpolate_subfaces(const std::vector<float> &cart)
	{
		std::vector<float> weights = point_to_barycentric(cart);
		std::vector<float> final_weights(weights.size(), 1);
		
		for (int i = 0; i < weights.size(); i++)
		{
			for (int j = 0; j < weights.size(); j++)
			{
				if (i == j)
				{
					continue;
				}

				final_weights[i] *= weights[j];
			}
		}

		Type total{}; float count = 0;

		for (int i = 0; i < pointCount(); i++)
		{
			SharedFace<N - 1, D, Type> *ls = faceExcluding(point(i));

			if (!ls) 
			{ 
				continue;
			}

			Type init = point(i)->value();
			Type end = ls->value_for_point(cart);
			float w = weights[i]; // weight of point
			float r = 1 - weights[i]; // weight of simplex
			float fw = final_weights[i];
			
			Type res = (w * init + r * end);
			total += res * fw;
			count += fw;
		}
		
		return total / count;
	}

	virtual Type value_for_point(const std::vector<float> &cart)
	{
		if (_variable)
		{
			return interpolate_variable(cart);
		}
		else
		{
			return interpolate_subfaces(cart);
		}
	}

	virtual std::vector<float> point_to_barycentric(const std::vector<float> &m)
	{
		std::vector<float> results(pointCount());
		
		for (size_t i = 0; i < D; i++)
		{
			_vec[i][0] = m.at(i);
		}

		_vec[D][0] = 1;
		
		if (_svd.u.cols != pointCount())
		{
			freeSVD(&_svd);
			setupSVD(&_svd, D+1, pointCount());
		}
		
		for (size_t row = 0; row < D; row++)
		{
			for (size_t col = 0; col < pointCount(); col++)
			{
				_svd.u[row][col] = point(col)->scalar(row);
			}
		}

		for (size_t col = 0; col < pointCount(); col++)
		{
			_svd.u[D][col] = 1;
		}
		
		invertSVD(&_svd);
		PCA::Matrix tr = transpose(&_svd.u);

		multMatrix(tr, _vec[0], &results[0]);

		return results;
	}

private:

	Type interpolate_variable(const std::vector<float> &cart)
	{
		std::vector<float> weights = point_to_barycentric(cart);
		Type val = _variable->interpolate_weights(weights);
		return val;
	}

	virtual size_t pointCount() const = 0;
	virtual const SharedFace<0, D, Type> *point(int idx) const = 0;
protected:
	PCA::Matrix _vec;
	PCA::SVD _svd;

	Variable<Type> *_variable = nullptr;
};

template <unsigned int D, typename Type>
class Interpolatable<0, D, Type> : public Mappable<Type>
{
public:
	virtual Type exact_value() = 0;

	virtual Type interpolate_subfaces(const std::vector<float> &cart)
	{
		return exact_value();
	}
	
	virtual Type value_for_point(const std::vector<float> &cart)
	{
		return exact_value();
	}
};

template <int N, unsigned int D, typename Type>
class SharedFace : public Interpolatable<N, D, Type>
{
public:
	typedef SharedFace<N - 1, D, Type> LowerFace;
	typedef SharedFace<N, D, Type> SameFace;
	
	virtual ~SharedFace<N, D, Type>() {}

	SharedFace(LowerFace *face, SharedFace<0, D, Type> &point) :
	SharedFace<N, D, Type>(*face, point) {}

	SharedFace(LowerFace &face, SharedFace<0, D, Type> &point)
	{
		this->subs().push_back(&face);
		_points = face.points();
		
		for (auto &lf : face.c_subs())
		{
			LowerFace *nf = new LowerFace(lf, point);
			this->subs().push_back(nf);
		}
		
		this->_points.push_back(&point);
	}
	
	virtual LowerFace *faceExcluding(const SharedFace<0, D, Type> *point)
	{
		for (auto &f : c_subs())
		{
			if (!f->hasPoint(*point))
			{
				return f;
			}
		}
		
		return nullptr;
	}
	
	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return _points[idx];
	}
	
	virtual SharedFace<0, D, Type> &v_point(int idx) const
	{
		return *_points[idx];
	}

	bool hasPoint(const SharedFace<0, D, Type> &point) const
	{
		for (SharedFace<0, D, Type> *p : _points)
		{
			if (p == &point)
			{
				return true;
			}
		}
		
		return false;
	}

	std::vector<SharedFace<0, D, Type> *> &points()
	{
		return _points;
	}

	virtual size_t pointCount() const
	{
		return _points.size();
	}
	
	size_t faceCount()
	{
		return _subs.size();
	}

	std::vector<LowerFace *> &subs() 
	{
		return _subs;
	}

	const std::vector<LowerFace *> &c_subs() const
	{
		return _subs;
	}

	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << N << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  *lf;
			i++;
		}
		return ss;
	}

protected:
	SharedFace() {};

	std::vector<LowerFace *> _subs;
	std::vector<SharedFace<0, D, Type> *> _points;

	int _n = N;
	int _d = D;
};

template <unsigned int D, typename Type>
class SharedFace<1, D, Type> : public Interpolatable<1, D, Type>
{
public:
	typedef SharedFace<1, D, Type> SameFace;

	SharedFace(SharedFace<0, D, Type> *face, SharedFace<0, D, Type> &point) :
	SharedFace<1, D, Type>(*face, point) {}

	SharedFace<1, D, Type>() {}
	virtual ~SharedFace<1, D, Type>() {}

	SharedFace(SharedFace<0, D, Type> &face, SharedFace<0, D, Type> &point)
	{
		_subs.push_back(&face);
		_subs.push_back(&point);
		_points.push_back(&face);
		_points.push_back(&point);
	}

	size_t pointCount() const
	{
		return _points.size();
	}
	
	std::vector<SharedFace<0, D, Type> *> &points()
	{
		return _points;
	}
	
	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return _points[idx];
	}

	virtual SharedFace<0, D, Type> &v_point(int idx) const
	{
		return *_points[idx];
	}

	virtual SharedFace<0, D, Type> *faceExcluding(const SharedFace<0, D, Type> 
	                                              *point)
	{
		for (SharedFace<0, D, Type> *f : c_subs())
		{
			if (f != point)
			{
				return f;
			}
		}
		
		return nullptr;
	}

	size_t faceCount() const
	{
		return _subs.size();
	}

	const std::vector<SharedFace<0, D, Type> *> &c_subs() const
	{
		return _subs;
	}

	std::vector<SharedFace<0, D, Type> *> &subs() 
	{
		return _subs;
	}

	bool hasPoint(const SharedFace<0, D, Type> &point) const
	{
		for (SharedFace<0, D, Type> *p : _points)
		{
			if (p == &point)
			{
				return true;
			}
		}
		
		return false;
	}


	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << 1 << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  *lf;
			i++;
		}
		return ss;
	}
private:
	std::vector<SharedFace<0, D, Type> *> _subs;
	std::vector<SharedFace<0, D, Type> *> _points;

	int _n = 0;
	int _d = D;
};

template <unsigned int D, typename Type>
class SharedFace<0, D, Type> : public Interpolatable<0, D, Type>, 
public Point<D, Type>
{
public:
	SharedFace() : Point<D, Type>() {};
	SharedFace(const std::vector<float> &p, Type value = 0) :
	Point<D, Type>(p, value) {}
	SharedFace(float *p, Type val) : Point<D, Type>(p, val) {};
	SharedFace(Point<D, Type> &p) : Point<D, Type>(p) {};
	SharedFace(SharedFace<0, D, Type> &face, SharedFace<0, D, Type> &other) {};

	virtual std::vector<float> point_to_barycentric(const std::vector<float> &m)
	{
		return std::vector<float>(1, 0);
	}

	virtual ~SharedFace<0, D, Type>() {}

	virtual Type exact_value()
	{
		return this->value();
	}

	virtual size_t pointCount() const
	{
		return 0;
	}

	size_t faceCount()
	{
		return 0;
	}

	std::vector<SharedFace<0, D, Type> *> points()
	{
		return std::vector<SharedFace<0, D, Type>>();
	}
	
	const std::vector<SharedFace<0, D, Type> *> c_subs() const
	{
		return std::vector<SharedFace<0, D, Type> *>();
	}

	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return nullptr;
	}
private:
	int _n = 0;
	int _d = D;
};


template <int N, unsigned int D, typename Type>
class Face : public SharedFace<N, D, Type>
{
public:
	typedef Face<N - 1, D, Type> LowerFace;
	typedef Face<N, D, Type> SameFace;

	Face(LowerFace &face, Face<0, D, Type> &point) :
	SharedFace<N, D, Type>(face, point) {};
	
	Face<N, D, Type>() {}

private:

};

template <int D, typename Type>
class Face<0, D, Type> : public SharedFace<0, D, Type>
{
public:
	Face() : SharedFace<0, D, Type>() {};
	Face(const std::vector<float> &p, Type value = 0) : 
	SharedFace<0, D, Type>(p, value) {};
	Face(float *p, Type val = Type{}) : SharedFace<0, D, Type>(p, val) {};
	Face(Point<D, Type> &p) : SharedFace<0, D, Type>(p) {};
	Face(const Face<0, D, Type> &face, const Face<0, D, Type> &other) {};

private:
	std::vector<SharedFace<0, D, Type> *> _points;

};

#endif
