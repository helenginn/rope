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

#include "CalcLayer.h"

void CalcLayer::makeFunction()
{
	_function = Function::functionFromType(_fType);
}

float *CalcLayer::allocateLocations()
{
	float *start = startPtr();

	_input = connectedLayer(-1)->outputLayerInfo();

	int m = connectedLayer(-1)->neuronCount();
	int n = neuronCount();
	_weights.populate(n, m, start);
	start += m * n;
	
	_bias = {start, neuronCount()};
	start += n;
	_sum = {start, neuronCount()};
	start += n;
	_eval = {start, neuronCount()};
	start += n;
	_grad = {start, neuronCount()};
	start += n;
	_sensitivities = {start, neuronCount()};
	start += n;
	return start;
}

size_t CalcLayer::requestedEntries()
{
	size_t mine = neuronCount();
	size_t before = connectedLayer(-1)->neuronCount();

	size_t weights = before * mine;
	size_t bias = mine;
	size_t sum = mine;
	size_t func_eval = mine;
	size_t func_grad = mine;
	size_t sensitivities = mine;
	
	size_t r = weights + bias + sum + func_eval + func_grad + sensitivities;
	return r;
}

void CalcLayer::learnTasks()
{
	std::vector<float> tmp(_weights.size());
	MatrixLoc wip;
	wip.populate(_weights.m, _weights.n, &tmp[0]);
	
	MatrixLoc::vec_by_transpose_vec(_sensitivities, _input, wip);
	
	if (_lType == ConstantAlpha)
	{
		wip *= _alpha;
		_sensitivities *= _alpha;
	}

	MatrixLoc::subtract_matrix(wip, _weights);
	VectorLoc::subtract_vector(_sensitivities, _bias);
	
}

void CalcLayer::initialiseWeights()
{
	if (_defaultWeights.size())
	{
		if (_defaultWeights.size() != _weights.size())
		{
			throw std::runtime_error("Default weights for layer don't match "\
			                         "required dimensions");
		}
		
		int count = 0;
		for (size_t i = 0; i < _weights.m; i++)
		{
			for (size_t j = 0; j < _weights.n; j++)
			{
				_weights[i][j] = _defaultWeights[count];
				count++;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < _weights.m; i++)
		{
			for (size_t j = 0; j < _weights.n; j++)
			{
				_weights[i][j] = rand() / (float)RAND_MAX - 0.5;
			}
		}
	}
	
	if (_defaultBiases.size())
	{
		if (_defaultBiases.size() != _bias.size)
		{
			throw std::runtime_error("Default biases for layer don't match "\
			                         "required dimensions.");
		}

		for (size_t i = 0; i < _bias.size; i++)
		{
			_bias[i] = _defaultBiases[i];
		}
	}
}
