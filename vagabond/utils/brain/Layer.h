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
#include "Locs.h"

class Layer
{
public:
	Layer();
	virtual ~Layer() {};

	void setNeuronCount(size_t n)
	{
		_nNeurons = n;
	}
	
	void setAlpha(float alpha)
	{
		_alpha = alpha;
	}
	
	std::vector<int> wantedLayers() const;
	
	void supplyLayer(int idx, Layer *other);
	
	/* number of floats wanted for total matrices, vector allocation
	 * by layer etc. */
	virtual size_t requestedEntries() = 0;
	
	void setStartPtr(float *ptr);
	virtual void connect() {};

	const size_t &neuronCount() const
	{
		return _nNeurons;
	}

	virtual const VectorLoc &outputLayerInfo() const = 0;

	virtual const MatrixLoc &weightInfo() const
	{
		return _nullMat;
	}

	virtual const VectorLoc &sensitivityInfo() const
	{
		return _nullVec;
	}
	
	void run();
	void back();
	void learn();
protected:
	void addLayerRequest(int idx);
	virtual void forwardTasks() {};
	virtual void backwardTasks() {};
	virtual void learnTasks() {};
	void setupIfNeeded();

	Layer *connectedLayer(int idx);

	virtual void furtherChecks() const {};
	void sanityCheck() const;

	virtual void setup() = 0;
	
	float *const startPtr() const
	{
		return _startPtr;
	}

	const VectorLoc _nullVec = {nullptr, 0};
	const MatrixLoc _nullMat = {nullptr, 0, 0};

	float _alpha = 0.1;
private:
	std::map<int, Layer *> _layerConnections;

	size_t _nNeurons = 0;
	bool _setup = false;
	
	float *_startPtr = nullptr;
};

#endif
