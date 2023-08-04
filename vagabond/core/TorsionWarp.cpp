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
		_coefficients.push_back(TDCoefficient(_parameters.size(), _nAxes, i));
	}
}

TorsionWarp::~TorsionWarp()
{
	for (TDCoefficient &coeff : _coefficients)
	{
		coeff.delete_matrix();
	}
}

std::vector<int> indicesForParameters(std::vector<Parameter *> &subset,
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

void TorsionWarp::getSetCoefficients(std::vector<Parameter *> &params,
                                     Getter &getter, Setter &setter,
                                     int max_dim)
{
	std::vector<int> indices = indicesForParameters(params, _parameters);

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

Floats TorsionWarp::torsions(const Coord::Get &get)
{
	Floats torsions;
	torsions.resize(_parameters.size());

	for (TDCoefficient &coeff : _coefficients)
	{
		for (size_t j = 0; j < _nAxes; j++)
		{
			Floats contributions = coeff.torsionContributions(j, get);
			torsions += contributions;
		}
	}
	
	return torsions;
}

Floats 
TorsionWarp::TDCoefficient::torsionContributions(int n, const Coord::Get &get)
{
	Floats result;
	result.resize(_nTorsions);
	multMatrix(_dim2Coeffs[n], get, &result[0]);
	
	for (size_t i = 0; i <= _order; i++)
	{
		result *= get(n);
	}
	
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

void TorsionWarp::filtersForParameter(Parameter *param, AtomFilter &left,
                                      AtomFilter &right)
{
	int id = param->residueId().as_num();
	left = [id](Atom *const &atom)
	{
		return (atom->atomName() == "CA" &&
		        (atom->residueId() > id - 3) &&
		        atom->residueId() < id + 3);
	};

	right = [id](Atom *const &atom)
	{
		return (atom->atomName() == "CA" &&
		        (atom->residueId() < id + 3) &&
		        (atom->residueId() > id - 3));
	};
}
