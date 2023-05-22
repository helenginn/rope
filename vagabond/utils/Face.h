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
class SharedFace
{
public:
	typedef SharedFace<N - 1, D> LowerFace;
	typedef SharedFace<N, D> SameFace;

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
	
	const SharedFace<0, D> &point(int idx) const
	{
		return *_points[idx];
	}

	size_t pointCount()
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
class SharedFace<1, D>
{
public:
	typedef SharedFace<1, D> SameFace;

	SharedFace(SharedFace<0, D> *face, SharedFace<0, D> &point) :
	SharedFace<1, D>(*face, point) {}

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
	
	const SharedFace<0, D> &point(int idx) const
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

	size_t pointCount()
	{
		return 0;
	}

	size_t faceCount()
	{
		return 0;
	}

	const std::vector<SharedFace<0, D>> c_subs() const
	{
		return std::vector<SharedFace<0, D>>();
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
	SharedFace<N, D>(face, point) {}
	
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

};

#endif
