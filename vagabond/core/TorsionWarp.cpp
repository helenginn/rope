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
	
	_coefficients.push_back(TDCoefficient(_parameters.size(), _nAxes, _nCoeff));
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
			TDCoefficient &coefficient = _coefficients[0];
			{
				PCA::Matrix &matrix = coefficient._nets[idx].first;
				for (int c = 0; c < matrix.cols * matrix.rows; c++)
				{
					values.push_back(matrix.vals[c]);
				}
			}
			{
				PCA::Matrix &matrix = coefficient._nets[idx].second;
				for (int c = 0; c < matrix.cols * matrix.rows; c++)
				{
					values.push_back(matrix.vals[c]);
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
			TDCoefficient &coefficient = _coefficients[0];
			{
				PCA::Matrix &matrix = coefficient._nets[idx].first;
				for (int c = 0; c < matrix.cols * matrix.rows; c++)
				{
					matrix.vals[c] = values[n] + start[n];
					n++;
				}
			}

			{
				PCA::Matrix &matrix = coefficient._nets[idx].second;
				for (int c = 0; c < matrix.cols * matrix.rows; c++)
				{
					matrix.vals[c] = values[n] + start[n];
					n++;
				}
			}
		}
	};
}

float TorsionWarp::torsion(const Coord::Get &get, int num)
{
	TDCoefficient &coefficient = _coefficients[0];
	PCA::Matrix &first = coefficient._nets[num].first;

	auto convert = [](float val) -> float
	{
		return 1 / (1 + exp(-val)) - 0.5;
	};

	float torsion = 0;
	PCA::Matrix &second = coefficient._nets[num].second;

	for (size_t k = 0; k < coefficient._order; k++)
	{
		float result = 0;
		for (size_t j = 0; j < coefficient._nDims; j++)
		{
			float get_axis = j == coefficient._nDims ? 1 : get(j);
			result += get_axis * first[k][j];
		}

		result = convert(result);
		torsion += second[0][k] * result;
	}
	
	return torsion;
}

void TorsionWarp::TDCoefficient::makeRandom()
{
	auto randomise = [](float) -> float
	{
		float r = rand() / (double)RAND_MAX - 0.5;
		return r;
	};

	for (size_t i = 0; i < _nTorsions; i++)
	{
		do_op(_nets[i].first, randomise);

		for (size_t o = 0; o < _order; o++)
		{
			_nets[i].first[o][_nDims] = 0;
		}

		do_op(_nets[i].second, randomise);
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
