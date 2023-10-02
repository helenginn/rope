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

#ifndef __vagabond__TorsionWarp__
#define __vagabond__TorsionWarp__

#include <set>
#include <vector>
#include <vagabond/utils/Vec3s.h>
#include <vagabond/utils/svd/PCA.h>

class Atom;
class Parameter;

class TorsionWarp
{
public:
	TorsionWarp(const std::vector<Parameter *> &parameterList,
	            int num_axes, int num_coeff);
	~TorsionWarp();
	
	void coefficientsFromJson(const json &j);

	float torsion(const Coord::Get &get, int num);
	
	typedef std::function<void(std::vector<float> &values)> Getter;
	typedef std::function<void(const std::vector<float> &values)> Setter;
	
	typedef std::function<bool(Atom *const &atom)> AtomFilter;

	void filtersForParameter(Parameter *param, AtomFilter &left,
	                         AtomFilter &right);
	void upToParameter(Parameter *param, AtomFilter &left,
	                   AtomFilter &right, bool reverse);

	void getSetCoefficients(const std::set<Parameter *> &params,
	                        Getter &getter, Setter &setter,
	                        int max_dim = INT_MAX);
private:
	
	struct TDCoefficient
	{
		TDCoefficient()
		{

		}
		
		struct Net
		{
			PCA::Matrix first{};
			PCA::Matrix second{};
		};

		TDCoefficient(int nTorsions, int nDims, int order)
		{
			for (size_t i = 0; i < nTorsions; i++)
			{
				Net nn{};
				setupMatrix(&nn.first, order, nDims + 1);
				setupMatrix(&nn.second, 1, order);
				_nets.push_back(nn);
			}

			_nDims = nDims;
			_nTorsions = nTorsions;
			_order = order;
			
			makeRandom();
		}
		
		void delete_matrix()
		{
			for (size_t i = 0; i < _nTorsions; i++)
			{
				freeMatrix(&_nets[i].first);
				freeMatrix(&_nets[i].second);
			}
		}
		
		void makeRandom();

		std::vector<Net> _nets;

		int _order = 0;
		int _nTorsions = 0;
		int _nDims = 0;

	};

	friend void from_json(const json &j, TDCoefficient &coeff);
	friend void from_json(const json &j, TDCoefficient::Net &net);
	
	friend void to_json(json &j, const TDCoefficient &coeff);
	friend void to_json(json &j, const TDCoefficient::Net &net);
	friend void to_json(json &j, const TorsionWarp &tw);

	std::vector<TDCoefficient> _coefficients;
	std::vector<Parameter *> _parameters;

	int _nCoeff = 1;
	int _nAxes = 0;
};
inline void from_json(const json &j, TorsionWarp::TDCoefficient::Net &net)
{
	net.first = j.at(0);
	net.second = j.at(1);
}

inline void to_json(json &j, const TorsionWarp::TDCoefficient::Net &net)
{
	j[0] = net.first;
	j[1] = net.second;

}

inline void from_json(const json &j, TorsionWarp::TDCoefficient &coeff)
{
	std::vector<TorsionWarp::TDCoefficient::Net> nets = j.at("nets");
	coeff._nets = nets;

	coeff._order = j.at("order");
	coeff._nTorsions = j.at("num_torsions");
	coeff._nDims = j.at("num_dims");

}

inline void to_json(json &j, const TorsionWarp::TDCoefficient &coeff)
{
	j["nets"] = coeff._nets;
	j["order"] = coeff._order;
	j["num_torsions"] = coeff._nTorsions;
	j["num_dims"] = coeff._nDims;

}

inline void to_json(json &j, const TorsionWarp &tw)
{
	j["coefficients"] = tw._coefficients;
}


#endif
