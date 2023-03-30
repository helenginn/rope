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

#include "Layer.h"
#include <string>

Layer::Layer()
{

}

std::vector<int> Layer::wantedLayers() const
{
	std::vector<int> list;
	
	for (auto it = _layerConnections.begin();
	     it != _layerConnections.end(); it++)
	{
		if (it->second == nullptr)
		{
			list.push_back(it->first);
		}
	}

	return list;
}

void Layer::supplyLayer(int idx, Layer *other)
{
	if (_layerConnections.count(idx) == 0)
	{
		throw std::runtime_error("Supplied layer is at unwanted index");
	}

	if (_layerConnections.at(idx) != nullptr)
	{
		throw std::runtime_error("Supplied layer has already been provided");
	}
	
	_layerConnections[idx] = other;
}

void Layer::addLayerRequest(int idx)
{
	_layerConnections[idx] = nullptr;
}

void Layer::setupIfNeeded()
{
	if (!_setup)
	{
		sanityCheck();
		setup();
		_setup = true;
	}
}

void Layer::sanityCheck() const
{
	for (auto it = _layerConnections.begin(); it != _layerConnections.end(); it++)
	{
		if (it->second == nullptr)
		{
			throw std::runtime_error("Layer connection missing: " +
			                         std::to_string(it->first));
		}
	}

	if (startPtr() == nullptr)
	{
		throw std::runtime_error("Layer requires float allocation before setup");
	}
	
	furtherChecks();
}
