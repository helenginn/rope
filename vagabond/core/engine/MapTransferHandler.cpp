// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "engine/MapTransferHandler.h"
#include "engine/workers/ThreadMapTransfer.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "ElementSegment.h"
#include "FFTCubicGrid.h"

MapTransferHandler::MapTransferHandler(BondCalculator *calculator)
{
	_calculator = calculator;
	_sumHandler = calculator->sumHandler();
}

MapTransferHandler::~MapTransferHandler()
{
	finish();
}

void MapTransferHandler::supplyAtomGroup(std::vector<Atom *> all)
{
	_all = all;
	
	getRealDimensions(_all);
}

void MapTransferHandler::getRealDimensions(std::vector<Atom *> &sub)
{
	_min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < sub.size(); i++)
	{
		glm::vec3 pos = sub[i]->derivedPosition();
		
		for (size_t j = 0; j < 3; j++)
		{
			_min[j] = std::min(_min[j], pos[j]);
			_max[j] = std::max(_max[j], pos[j]);
		}
	}
	
	if (sub.size() == 0)
	{
		_min = glm::vec3(0.);
		_max = glm::vec3(0.);
	}
	
	_min -= _pad;
	_max += _pad;
}

void MapTransferHandler::supplyElementList(std::map<std::string, int> elements)
{
	std::map<std::string, int>::iterator it;
	for (it = elements.begin(); it != elements.end(); it++)
	{
		_elements.push_back(it->first);
	}

}

void MapTransferHandler::allocateSegments()
{
	int nx, ny, nz;
	ElementSegment::findDimensions(nx, ny, nz, _min, _max, _cubeDim);
	
	for (size_t i = 0; i < _elements.size(); i++)
	{
		std::string &ele = _elements[i];
		Pool<ElementSegment *> &pool = _pools[ele];

		for (size_t j = 0; j < _mapNum; j++)
		{
			ElementSegment *seg = new ElementSegment();
			seg->setDimensions(nx, ny, nz);
			seg->setRealDim(_cubeDim);
			seg->setOrigin(_min);
			seg->setElement(ele);
			seg->setStatus(FFT<VoxelElement>::Real);
			seg->makePlans();

			_segments.push_back(seg);
			pool.pushObject(seg);
		}
	}
}

void MapTransferHandler::setup()
{
	allocateSegments();
}

void MapTransferHandler::prepareThreads()
{
	for (size_t j = 0; j < _elements.size(); j++)
	{
		Pool<ElementSegment *> &pool = _pools[_elements[j]];

		for (size_t i = 0; i < _threads; i++)
		{
			/* several calculators */
			ThreadMapTransfer *worker = new ThreadMapTransfer(this, _elements[j]);
			worker->setMapSumHandler(_sumHandler);
			worker->setPointStoreHandler(_pointHandler);
			std::thread *thr = new std::thread(&ThreadMapTransfer::start, worker);

			pool.threads.push_back(thr);
			pool.workers.push_back(worker);
		}
	}
}

void MapTransferHandler::start()
{
	_finish = false;

	prepareThreads();
}

void MapTransferHandler::returnSegment(ElementSegment *segment)
{
	segment->clear();
	std::string ele = segment->elementSymbol();
	Pool<ElementSegment *> &pool = _pools[ele];
	pool.pushObject(segment);
}

ElementSegment *MapTransferHandler::acquireSegment(std::string ele)
{
	Pool<ElementSegment *> &pool = _pools[ele];
	ElementSegment *seg = nullptr;
	pool.acquireObject(seg, _finish);
	return seg;
}

void MapTransferHandler::signalThreads()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		_pools[_elements[i]].signalThreads();
	}
}

void MapTransferHandler::joinThreads()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		_pools[_elements[i]].joinThreads();
	}

	for (size_t i = 0; i < _elements.size(); i++)
	{
		_pools[_elements[i]].cleanup();
	}
}

void MapTransferHandler::finish()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		_pools[_elements[i]].handout.lock();
	}
	
	_finish = true;

	for (size_t i = 0; i < _elements.size(); i++)
	{
		_pools[_elements[i]].handout.unlock();
	}

	signalThreads();
}
