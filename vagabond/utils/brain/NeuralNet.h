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

#ifndef __vagabond__NeuralNet__
#define __vagabond__NeuralNet__

/** really simple implementation of neural network with input, hidden
 * and output layers; backpropagation */

#include "InputLayer.h"
#include "HiddenLayer.h"
#include "OutputLayer.h"

class NeuralNet
{
public:
	NeuralNet();

	void setInputOutput(float *input, float *output);

	void runAndLearn();

	void connect();
	void learnRun();
	void forwardRun();
	void backwardRun();

	InputLayer &inputLayer()
	{
		return _inputLayer;
	}

	OutputLayer &outputLayer()
	{
		return _outputLayer;
	}
	
	void addLayer(HiddenLayer &other)
	{
		if (_connected)
		{
			throw std::runtime_error("Already connected neural net, "\
			                         "no more layers");
		}

		_hiddenLayers.push_back(other);
		HiddenLayer *hidden = &_hiddenLayers.back();
		_middleLayers.push_back(hidden);
	}
	
	const std::vector<float> &residuals() const
	{
		return _residuals;
	}
	
	Layer *layer(int i)
	{
		return _allLayers[i];
	}
private:
	void makeLayerList();
	void provideLayers();
	void acquireSpace();

	void saveResidual();

	InputLayer _inputLayer{};
	OutputLayer _outputLayer{};

	std::vector<HiddenLayer> _hiddenLayers;
	std::vector<Layer *> _middleLayers;
	std::vector<Layer *> _allLayers;
	
	bool _connected = false;
	std::vector<float> _workingArea;
	std::vector<float> _residuals;
};

#endif
