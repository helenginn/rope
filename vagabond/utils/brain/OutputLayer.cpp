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

#include "OutputLayer.h"
#include <cmath>

OutputLayer::OutputLayer() : CalcLayer()
{
	addLayerRequest(-1);
}

float *OutputLayer::allocateLocations()
{
	float *ptr = CalcLayer::allocateLocations();
	size_t n = neuronCount();
	_targets = {ptr, n};
	return ptr;
}

void OutputLayer::expectOutput(float *first)
{
	for (size_t i = 0; i < _targets.size; i++)
	{
		_targets.ptr[i] = first[i];
	}
}

void OutputLayer::setup()
{
	allocateLocations();
	initialiseWeights();
	makeFunction();
}

size_t OutputLayer::requestedEntries()
{
	size_t mine = neuronCount();
	size_t targets = mine;
	
	return CalcLayer::requestedEntries() + targets;
}

void OutputLayer::forwardTasks()
{
	MatrixLoc::multiply(_weights, _input, _sum);
	VectorLoc::add_vector(_bias, _sum);
	function()->evaluate(_sum, _eval);
	function()->gradient(_sum, _grad);
}

void OutputLayer::calculateResidual()
{
	float nom = 0;
	for (size_t i = 0; i < _sensitivities.size; i++)
	{
		nom += _sensitivities[i] * _sensitivities[i];
	}
	
	_residual = sqrt(nom) / (float)_sensitivities.size;
}

void OutputLayer::backwardTasks()
{
	// sensitivities = target minus evaluation
	VectorLoc::copy_vector(_targets, _sensitivities);
	VectorLoc::subtract_vector(_eval, _sensitivities);
	
	calculateResidual();
	
	// pairwise multiplication with gradients
	VectorLoc::dot_vector(_sensitivities, _grad);
	_sensitivities *= -2;
}

std::ostream &operator<<(std::ostream &ss, const OutputLayer *o)
{
	ss << "OutputLayer" << std::endl;
	ss << "[-1] Input: " << o->_input << std::endl;
	ss << "     Weights: " << o->_weights << std::endl;
	ss << "     Biases: " << o->_bias << std::endl;
	ss << "     Sum: " << o->_sum << std::endl;
	ss << "     Eval: " << o->_eval << std::endl;
	ss << "     Grad: " << o->_grad << std::endl;
	ss << "     Sensitivities: " << o->_sensitivities << std::endl;
	return ss;
}
