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

#ifndef __vagabond__CalcLayer__
#define __vagabond__CalcLayer__

#include "Layer.h"
#include "Function.h"

class CalcLayer : public Layer
{
public:
	virtual const VectorLoc &outputLayerInfo() const
	{
		return _eval;
	}
	
	virtual const VectorLoc &sensitivityInfo() const
	{
		return _sensitivities;
	}

	virtual const MatrixLoc &weightInfo() const
	{
		return _weights;
	}

	virtual void setDefaultBiases(std::vector<float> &biases)
	{
		_defaultBiases = biases;
	}

	virtual void setDefaultWeights(std::vector<float> &weights)
	{
		_defaultWeights = weights;
	}
	
	void setLearningType(LearningType lt)
	{
		_lType = lt;
	}
	
	void setFunctionType(FunctionType ft)
	{
		_fType = ft;
	}

	virtual size_t requestedEntries();
protected:
	virtual void learnTasks();
	virtual float *allocateLocations();
	void initialiseWeights();
	void makeFunction();

	const Function *function() const
	{
		return _function;
	}

	VectorLoc _grad = {nullptr, 0};
	VectorLoc _input = {nullptr, 0};
	VectorLoc _eval = {nullptr, 0};
	MatrixLoc _weights = {nullptr, 0, 0};
	VectorLoc _bias = {nullptr, 0};
	VectorLoc _sum = {nullptr, 0};
	VectorLoc _sensitivities = {nullptr, 0};
private:
	FunctionType _fType = FPureLinear;
	LearningType _lType = ConstantAlpha;

	Function *_function = nullptr;
	
	std::vector<float> _defaultWeights;
	std::vector<float> _defaultBiases;
};

#endif

