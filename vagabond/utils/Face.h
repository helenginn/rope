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

#include "svd/PCA.h"
#include "Variable.h"

template <unsigned int D>
class Point
{
public:
	Point() {}
	Point(float *p, float value = 0)
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
	
	const float &value() const
	{
		return _value;
	}
private:
	float _p[D]{};
	float _value{};

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
		ss << ")" << std::endl;
		return ss;
	}
};

template <int N, unsigned int D>
class SharedFace;

template <int N, unsigned int D>
class Interpolatable
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
	
	void setVariable(Variable *var)
	{
		_variable = var;
	}

	virtual SharedFace<N - 1, D> *faceExcluding(const SharedFace<0, D> &point)
	{
		return nullptr;
	}

	float interpolate_subfaces(Point<D> &cart)
	{
		std::vector<float> weights = point_to_barycentric(cart);

		float total = 0; float count = 0;
		for (int i = 0; i < pointCount(); i++)
		{
			SharedFace<N - 1, D> *ls = faceExcluding(point(i));

			if (!ls) { continue; }

			float init = point(i).value();
			float end = ls->value_for_point(cart);
			float w = weights[i]; // weight of point
			float r = 1 - weights[i]; // weight of simplex
			
			float res = (w * init + r * end) / (w + r);
			total += res;
			count++;
		}
		
		return total / count;
	}

	virtual float value_for_point(Point<D> cart)
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

private:

	std::vector<float> point_to_barycentric(Point<D> &middle)
	{
		std::vector<float> results(pointCount());
		
		for (size_t i = 0; i < D; i++)
		{
			_vec[i][0] = middle.p()[i];
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
				_svd.u[row][col] = point(col).scalar(row);
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

	float interpolate_variable(Point<D> &cart)
	{
		std::vector<float> weights = point_to_barycentric(cart);
		float val = _variable->interpolate_weights(weights);
		return val;
	}
	

	virtual size_t pointCount() const = 0;
	virtual const SharedFace<0, D> &point(int idx) const = 0;
protected:
	PCA::Matrix _vec;
	PCA::SVD _svd;

	Variable *_variable = nullptr;
};


template <int N, unsigned int D>
class SharedFace : public Interpolatable<N, D>
{
public:
	typedef SharedFace<N - 1, D> LowerFace;
	typedef SharedFace<N, D> SameFace;
	
	virtual ~SharedFace<N, D>() {}

	SharedFace(LowerFace *face, SharedFace<0, D> &point) :
	SharedFace<N, D>(*face, point) {}

	SharedFace(LowerFace &face, SharedFace<0, D> &point)
	{
		this->subs().push_back(&face);
		_points = face._points;
		
		for (auto &lf : face.c_subs())
		{
			LowerFace *nf = new LowerFace(lf, point);
			this->subs().push_back(nf);
		}
		
		this->_points.push_back(&point);
	}
	
	LowerFace *faceExcluding(const SharedFace<0, D> &point)
	{
		for (auto &f : c_subs())
		{
			if (!f->hasPoint(point))
			{
				return f;
			}
		}
		
		return nullptr;
	}
	
	virtual const SharedFace<0, D> &point(int idx) const
	{
		return *_points[idx];
	}
	
	bool hasPoint(const SharedFace<0, D> &point) const
	{
		for (SharedFace<0, D> *p : _points)
		{
			if (p == &point)
			{
				return true;
			}
		}
		
		return false;
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
	std::vector<SharedFace<0, D> *> _points;

	int _n = N;
	int _d = D;
};

template <unsigned int D>
class SharedFace<1, D> : public Interpolatable<1, D>
{
public:
	typedef SharedFace<1, D> SameFace;

	SharedFace(SharedFace<0, D> *face, SharedFace<0, D> &point) :
	SharedFace<1, D>(*face, point) {}

	SharedFace<1, D>() {}
	virtual ~SharedFace<1, D>() {}

	SharedFace(SharedFace<0, D> &face, SharedFace<0, D> &point)
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
	
	virtual const SharedFace<0, D> &point(int idx) const
	{
		return *_points[idx];
	}

	size_t faceCount() const
	{
		return _subs.size();
	}

	const std::vector<SharedFace<0, D> *> &c_subs() const
	{
		return _subs;
	}

	std::vector<SharedFace<0, D> *> &subs() 
	{
		return _subs;
	}

	bool hasPoint(const SharedFace<0, D> &point) const
	{
		for (SharedFace<0, D> *p : _points)
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
	std::vector<SharedFace<0, D> *> _subs;
	std::vector<SharedFace<0, D> *> _points;

	int _n = 0;
	int _d = D;
};

template <unsigned int D>
class SharedFace<0, D> : public Point<D>
{
public:
	SharedFace() : Point<D>() {};
	SharedFace(float *p, float val) : Point<D>(p, val) {};
	SharedFace(SharedFace<0, D> &face, SharedFace<0, D> &other) {};

	virtual float value_for_point(Point<D> cart)
	{
		return this->value();
	}

	virtual ~SharedFace<0, D>() {}

	virtual size_t pointCount() const
	{
		return 0;
	}

	size_t faceCount()
	{
		return 0;
	}

	const std::vector<SharedFace<0, D> *> c_subs() const
	{
		return std::vector<SharedFace<0, D> *>();
	}
private:
	int _n = 0;
	int _d = D;
};


template <int N, unsigned int D>
class Face : public SharedFace<N, D>
{
public:
	typedef Face<N - 1, D> LowerFace;
	typedef Face<N, D> SameFace;

	Face(LowerFace &face, Face<0, D> &point) :
	SharedFace<N, D>(face, point) {};
	
	Face<N, D>() {}

private:

};

template <int D>
class Face<0, D> : public SharedFace<0, D>
{
public:
	Face() : SharedFace<0, D>() {};
	Face(float *p, float val = 0) : SharedFace<0, D>(p, val) {};
	Face(const Face<0, D> &face, const Face<0, D> &other) {};

private:
	std::vector<SharedFace<0, D> *> _points;

};

#endif
