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

		TDCoefficient(int nTorsions, int nDims, int order)
		{
			for (size_t i = 0; i < nDims; i++)
			{
				PCA::Matrix next;
				setupMatrix(&next, nTorsions, nDims);
				_dim2Coeffs.push_back(next);
			}
			_nDims = nDims;
			_nTorsions = nTorsions;
			_order = order;
		}
		
		void delete_matrix()
		{
			for (size_t i = 0; i < _nDims; i++)
			{
				freeMatrix(&_dim2Coeffs[i]);
			}
		}
		
		void makeRandom();
		
		Floats torsionContributions(int n, const Coord::Get &get, int num);
		float torsionContribution(int n, const Coord::Get &get, int num);

		std::vector<PCA::Matrix> _dim2Coeffs;

		int _order = 0;
		int _nTorsions = 0;
		int _nDims = 0;

	};

	friend void from_json(const json &j, TDCoefficient &coeff);
	
	friend void to_json(json &j, const TDCoefficient &coeff);
	friend void to_json(json &j, const TorsionWarp &tw);

	std::vector<TDCoefficient> _coefficients;
	std::vector<Parameter *> _parameters;

	int _nCoeff = 1;
	int _nAxes = 0;
};

inline void from_json(const json &j, TorsionWarp::TDCoefficient &coeff)
{
	std::vector<PCA::Matrix> coeffs = j.at("matrices");
	coeff._dim2Coeffs = coeffs;

	coeff._order = j.at("order");
	coeff._nTorsions = j.at("num_torsions");
	coeff._nDims = j.at("num_dims");

}

inline void to_json(json &j, const TorsionWarp::TDCoefficient &coeff)
{
	j["matrices"] = coeff._dim2Coeffs;
	j["order"] = coeff._order;
	j["num_torsions"] = coeff._nTorsions;
	j["num_dims"] = coeff._nDims;

}

inline void to_json(json &j, const TorsionWarp &tw)
{
	j["coefficients"] = tw._coefficients;
}


#endif
