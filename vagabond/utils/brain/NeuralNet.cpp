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

#include "NeuralNet.h"

NeuralNet::NeuralNet()
{

}

void NeuralNet::makeLayerList()
{
	_allLayers.push_back(&_inputLayer);
	
	for (Layer *l : _middleLayers)
	{
		_allLayers.push_back(l);
	}
	
	_allLayers.push_back(&_outputLayer);

}

void NeuralNet::provideLayers()
{
	for (int i = 0; i < _allLayers.size(); i++)
	{
		Layer *curr = _allLayers[i];
		
		std::vector<int> requests = curr->wantedLayers();
		
		for (int req : requests)
		{
			int idx = req + i;
			
			if (idx < 0)
			{
				throw std::runtime_error("Cannot request layer, negative index");
			}
			if (idx >= _allLayers.size())
			{
				throw std::runtime_error("Cannot request layer, "\
				                         "index beyond bounds" );
			}

			Layer *found = _allLayers[idx];
			curr->supplyLayer(req, found);
		}
	}
}

void NeuralNet::acquireSpace()
{
	std::map<Layer *, size_t> locations;

	size_t running_total = 0;
	for (int i = 0; i < _allLayers.size(); i++)
	{
		Layer *curr = _allLayers[i];
		size_t request = curr->requestedEntries();
		
		locations[curr] = running_total;
		running_total += request;
	}
	
	_workingArea.resize(running_total);
	float *track = &_workingArea[0];
	
	for (int i = 0; i < _allLayers.size(); i++)
	{
		Layer *curr = _allLayers[i];
		int loc = locations[curr];
		track = &_workingArea[0] + loc;
		curr->setStartPtr(track);
	}

	for (int i = 0; i < _allLayers.size(); i++)
	{
		_allLayers[i]->connect();
	}
}

void NeuralNet::connect()
{
	makeLayerList();
	provideLayers();
	acquireSpace();

	_connected = true;
}

void NeuralNet::forwardRun()
{
	for (Layer *layer : _allLayers)
	{
		layer->run();
	}
}

void NeuralNet::backwardRun()
{
	for (int i = _allLayers.size() - 1; i >= 0; i--)
	{
		Layer *layer = _allLayers[i];
		layer->back();
	}
}

void NeuralNet::saveResidual()
{
	float res = outputLayer().residual();
	_residuals.push_back(res);
}

void NeuralNet::learnRun()
{
	saveResidual();

	for (int i = _allLayers.size() - 1; i >= 0; i--)
	{
		Layer *layer = _allLayers[i];
		layer->learn();
	}
}

void NeuralNet::runAndLearn()
{
	forwardRun();
	backwardRun();
	learnRun();
}

void NeuralNet::setInputOutput(float *input, float *output)
{
	inputLayer().enterInput(input);
	outputLayer().expectOutput(output);
}
