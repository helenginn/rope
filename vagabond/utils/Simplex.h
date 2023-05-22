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
	Point(float *p)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
	}
private:
	float _p[D]{};

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

	SharedFace(const LowerFace &face, const SharedFace<0, D> &point)
	{
		this->subs().push_back(face);
		
		for (auto &lf : face.c_subs())
		{
			LowerFace nf = LowerFace(lf, point);
			this->subs().push_back(nf);
		}
	}
	
	size_t faceCount()
	{
		return _subs.size();
	}

	std::vector<LowerFace> &subs() 
	{
		return _subs;
	}

	const std::vector<LowerFace> &c_subs() const
	{
		return _subs;
	}

	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << N << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  lf;
			i++;
		}
		return ss;
	}
protected:
	SharedFace() {};

	std::vector<LowerFace> _subs;

	int _n = N;
	int _d = D;
};

template <unsigned int D>
class SharedFace<1, D>
{
public:
	typedef SharedFace<1, D> SameFace;

	SharedFace(const SharedFace<0, D> &face, const SharedFace<0, D> &point)
	{
		_subs.push_back(face);
		_subs.push_back(point);
	}

	size_t faceCount()
	{
		return _subs.size();
	}

	const std::vector<SharedFace<0, D>> &c_subs() const
	{
		return _subs;
	}

	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << 1 << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  lf;
			i++;
		}
		return ss;
	}
private:
	std::vector<SharedFace<0, D>> _subs;

	int _n = 0;
	int _d = D;
};

template <unsigned int D>
class SharedFace<0, D> : public Point<D>
{
public:
	SharedFace() : Point<D>() {};
	SharedFace(float *p) : Point<D>(p) {};
	SharedFace(const SharedFace<0, D> &face, const SharedFace<0, D> &other) {};

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

	Face(const LowerFace &face, const Face<0, D> &point) :
	SharedFace<N, D>(face, point) {}
	
private:

};

template <int D>
class Face<0, D> : public SharedFace<0, D>
{
public:
	Face() : SharedFace<0, D>() {};
	Face(float *p) : SharedFace<0, D>(p) {};
	Face(const Face<0, D> &face, const Face<0, D> &other) {};

private:

};

#endif
