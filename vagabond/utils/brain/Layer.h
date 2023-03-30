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

#ifndef __vagabond__Layer__
#define __vagabond__Layer__

#include <vector>
#include <map>

class Layer
{
public:
	Layer();
	virtual ~Layer() {};

	void setNeuronCount(size_t n)
	{
		_nNeurons = n;
	}
	
	std::vector<int> wantedLayers() const;
	
	void supplyLayer(int idx, Layer *other);
	
	/* number of floats wanted for total matrices, vector allocation
	 * by layer etc. */
	virtual size_t requestedEntries() = 0;
	
	void setStartPtr(float *ptr)
	{
		_startPtr = ptr;
	}

	const size_t &neuronCount() const
	{
		return _nNeurons;
	}
protected:
	void addLayerRequest(int idx);

	virtual void furtherChecks() const {};
	void sanityCheck() const;

	virtual void setup() = 0;
	void setupIfNeeded();
	
	float *const startPtr() const
	{
		return _startPtr;
	}
	
	struct VectorLoc
	{
		float *ptr;
		size_t size;
	};
private:
	std::map<int, Layer *> _layerConnections;

	size_t _nNeurons = 0;
	bool _setup = false;
	
	float *_startPtr = nullptr;
};

#endif
