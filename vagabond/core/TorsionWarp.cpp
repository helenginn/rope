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

#include "TorsionWarp.h"
#include "Parameter.h"
#include "Atom.h"

TorsionWarp::TorsionWarp(const std::vector<Parameter *> &parameterList,
                         int num_axes, int num_coeff)
{
	_parameters = parameterList;
	_nAxes = num_axes;
	_nCoeff = num_coeff;
	
	for (size_t i = 0; i < _nCoeff; i++)
	{
		_coefficients.push_back(TDCoefficient(_parameters.size(), _nAxes, i+1));
	}
}

TorsionWarp::~TorsionWarp()
{
	for (TDCoefficient &coeff : _coefficients)
	{
		coeff.delete_matrix();
	}
}

std::vector<int> indicesForParameters(const std::set<Parameter *> &subset,
                                      std::vector<Parameter *> &all)
{
	std::vector<int> idxs;

	for (Parameter *p : subset)
	{
		for (size_t j = 0; j < all.size(); j++)
		{
			if (all[j] == p)
			{
				idxs.push_back(j);
			}
		}
	}
	
	return idxs;
}

void TorsionWarp::getSetCoefficients(const std::set<Parameter *> &params,
                                     Getter &getter, Setter &setter,
                                     int max_dim)
{
	std::vector<int> indices = indicesForParameters(params, _parameters);
	std::sort(indices.begin(), indices.end());
	
	getter = [indices, max_dim, this](std::vector<float> &values)
	{
		for (const int &idx : indices)
		{
			for (TDCoefficient &coefficient : _coefficients)
			{
				for (PCA::Matrix &matrix : coefficient._dim2Coeffs)
				{
					for (int c = 0; c < matrix.cols && c < max_dim; c++)
					{
						values.push_back(matrix[idx][c]);
					}
				}
			}
		}
	};
	
	std::vector<float> start;
	getter(start);

	setter = [indices, max_dim, start, this](const std::vector<float> &values)
	{
		int n = 0;
		for (const int &idx : indices)
		{
			for (TDCoefficient &coefficient : _coefficients)
			{
				for (PCA::Matrix &matrix : coefficient._dim2Coeffs)
				{
					for (int c = 0; c < matrix.cols && c < max_dim; c++)
					{
						matrix[idx][c] = values[n] + start[n];
						n++;
					}
				}
			}
		}
	};
}

float TorsionWarp::torsion(const Coord::Get &get, int num)
{
	float torsion = 0;
	
	for (TDCoefficient &coeff : _coefficients)
	{
		for (size_t j = 0; j < _nAxes; j++)
		{
			torsion += coeff.torsionContribution(j, get, num);
		}
	}
	
	return torsion;
}

float 
TorsionWarp::TDCoefficient::torsionContribution(int n, const Coord::Get &get, 
                                                 int num)
{
	float result = 0;
	
	PCA::Matrix &coeffs = _dim2Coeffs[n];
	for (size_t i = 0; i < coeffs.cols; i++)
	{
		result += coeffs[num][i] * get(i);
	}
	
	float mult = 1;
	float get_axis = get(n);
	for (size_t i = 0; i <= _order; i++)
	{
		mult *= get_axis;
	}

	result *= mult;
	
	return result;
}

Floats 
TorsionWarp::TDCoefficient::torsionContributions(int n, const Coord::Get &get, 
                                                 int num)
{
	Floats result;
	result.resize(num < 0 ? _nTorsions : 1);
	
	PCA::Matrix &coeffs = _dim2Coeffs[n];
	if (num < 0)
	{
		multMatrix(coeffs, get, &result[0]);
	}
	else
	{
		result[0] = 0;
		for (size_t i = 0; i < coeffs.cols; i++)
		{
			result[0] += coeffs[num][i] * get(i);
		}
	}
	
	float mult = 1;
	for (size_t i = 0; i <= _order; i++)
	{
		mult *= get(n);
	}

	result *= mult;
	
	return result;
}

void TorsionWarp::TDCoefficient::makeRandom()
{
	auto randomise = [](float) -> float
	{
		float r = rand() / (double)RAND_MAX - 0.5;
		return r;
	};

	for (size_t i = 0; i < _nDims; i++)
	{
		do_op(_dim2Coeffs[i], randomise);
	}
}

void TorsionWarp::upToParameter(Parameter *param, AtomFilter &left,
                                AtomFilter &right, bool reverse)
{
	int id = param->residueId().as_num();
	if (!reverse)
	{
		left = [id](Atom *const &atom)
		{
			return (atom->atomName() == "CA" &&
			        atom->residueId() < id + 4);
		};

		right = [id](Atom *const &atom)
		{
			return (atom->atomName() == "CA" &&
			        (atom->residueId() < id + 4));
		};
	}
	else
	{
		left = [id](Atom *const &atom)
		{
			return (atom->atomName() == "CA" &&
			        atom->residueId() > id - 4);
		};

		right = [id](Atom *const &atom)
		{
			return (atom->atomName() == "CA" &&
			        (atom->residueId() > id - 4));
		};
	}
}

void TorsionWarp::filtersForParameter(Parameter *param, AtomFilter &left,
                                      AtomFilter &right)
{
	int id = param->residueId().as_num();
	left = [id](Atom *const &atom)
	{
		return ((atom->residueId() > id - 4) &&
		        atom->residueId() < id + 4);
	};

	right = [id](Atom *const &atom)
	{
		return ((atom->residueId() < id + 4) &&
		        (atom->residueId() > id - 4));
	};
}

void TorsionWarp::coefficientsFromJson(const json &j)
{
	std::vector<TDCoefficient> coefficients = j.at("coefficients");
	_coefficients = coefficients;
}
