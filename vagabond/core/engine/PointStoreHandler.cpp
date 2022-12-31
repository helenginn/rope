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

#include "engine/PointStoreHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/PointStore.h"

PointStoreHandler::PointStoreHandler(BondCalculator *calculator)
{
	_calculator = calculator;
}

PointStoreHandler::~PointStoreHandler()
{
	// cleanup

	for (size_t i = 0; i < _elements.size(); i++)
	{
		_loadedPool[_elements[i]].cleanup();
	}

	_emptyPool.cleanup();
}

void PointStoreHandler::loadMixedPositions(Job *job, 
                                           std::vector<BondSequence::ElePos> epos)
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		PointStore *ps = acquireEmptyStore();
		ps->loadPositions(_elements[i], epos);
		ps->setJob(job);
		returnFilledStore(ps);
	}
}

void PointStoreHandler::setMapHandler(MapTransferHandler *handler)
{
	_mapHandler = handler;
}

void PointStoreHandler::allocateStores()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		for (size_t j = 0; j < _storeCount; j++)
		{
			PointStore *ps = new PointStore();
			_emptyPool.pushObject(ps);
		}
		_loadedPool[_elements[i]].sem.setName("loaded");
	}

}

void PointStoreHandler::setup()
{
	_elements = _mapHandler->elements();
	allocateStores();
}

void PointStoreHandler::start()
{
	_finish = false;
}

PointStore *PointStoreHandler::acquireEmptyStore()
{
	Pool<PointStore *> &pool = _emptyPool;
	PointStore *ps = nullptr;
	pool.acquireObject(ps, _finish);

	return ps;

}

PointStore *PointStoreHandler::acquireFilledStore(std::string ele)
{
	Pool<PointStore *> &pool = _loadedPool[ele];
	PointStore *ps = nullptr;
	pool.acquireObject(ps, _finish);

	return ps;
}

void PointStoreHandler::returnFilledStore(PointStore *ps)
{
	std::string ele = ps->element();
	_loadedPool[ele].pushObject(ps);
}

void PointStoreHandler::returnEmptyStore(PointStore *ps)
{
	ps->clear();
	_emptyPool.pushObject(ps);
}

void PointStoreHandler::signalThreads()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		Pool<PointStore *> &pool = _loadedPool[_elements[i]];
		pool.signalThreads();
		_emptyPool.signalThreads();
	}
}

void PointStoreHandler::finish()
{
	// join threads
	
	for (size_t i = 0; i < _elements.size(); i++)
	{
		_loadedPool[_elements[i]].handout.lock();
	}
	
	_emptyPool.handout.lock();

	_finish = true;

	for (size_t i = 0; i < _elements.size(); i++)
	{
		_loadedPool[_elements[i]].handout.unlock();
	}

	_emptyPool.handout.unlock();

	signalThreads();
}
