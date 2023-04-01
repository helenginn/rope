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

#include "HiddenLayer.h"

HiddenLayer::HiddenLayer() : CalcLayer()
{
	addLayerRequest(-1);
	addLayerRequest(+1);
}

void HiddenLayer::setup()
{
	allocateLocations();
	initialiseWeights();
	makeFunction();
}

void HiddenLayer::connect()
{
	_nextSensitivities = connectedLayer(+1)->sensitivityInfo();
	_nextWeights = connectedLayer(+1)->weightInfo();
}

size_t HiddenLayer::requestedEntries()
{
	return CalcLayer::requestedEntries();
}

void HiddenLayer::forwardTasks()
{
	MatrixLoc::multiply(_weights, _input, _sum);
	VectorLoc::add_vector(_bias, _sum);
	
	function()->evaluate(_sum, _eval);
	function()->gradient(_eval, _grad);
}

void HiddenLayer::backwardTasks()
{
	MatrixLoc::multiply_transpose(_nextWeights, _nextSensitivities, 
	                              _sensitivities);
	VectorLoc::dot_vector(_grad, _sensitivities);
}

std::ostream &operator<<(std::ostream &ss, const HiddenLayer *h)
{
	ss << "Hidden layer" << std::endl;
	ss << "[-1] Input: " << h->_input << std::endl;
	ss << "     Weights: " << h->_weights << std::endl;
	ss << "     Bias: " << h->_bias << std::endl;
	ss << "     Sum: " << h->_sum << std::endl;
	ss << "     Eval: " << h->_eval << std::endl;
	ss << "     Grad: " << h->_grad << std::endl;
	ss << "     Sensitivities: " << h->_sensitivities << std::endl;
	return ss;
}
