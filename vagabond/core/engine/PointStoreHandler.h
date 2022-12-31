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

#ifndef __vagabond__PointStoreHandler__
#define __vagabond__PointStoreHandler__

#include "engine/Handler.h"
#include "BondSequence.h"

class BondCalculator;
class MapTransferHandler;
class PointStore;
struct Job;

class PointStoreHandler : public Handler
{
public:
	PointStoreHandler(BondCalculator *calculator = nullptr);
	~PointStoreHandler();

	void setMapHandler(MapTransferHandler *handler);
	
	void setThreads(int threads)
	{
		_threads = threads;
	}
	
	void loadMixedPositions(Job *job, std::vector<BondSequence::ElePos> epos);

	/** grab an object capable of taking in new points */
	PointStore *acquireEmptyStore();

	/** grab an object with points that needs to be unloaded into an
	 * ElementSegment
	 * @param ele element symbol for ElementSegment */
	PointStore *acquireFilledStore(std::string ele);

	/** return a filled point store */
	void returnFilledStore(PointStore *ps);

	/** return an empty point store to the general pool */
	void returnEmptyStore(PointStore *ps);

	void setup();

	void start();
	void finish();
private:
	void allocateStores();
	void signalThreads();
	void prepareThreads();

	BondCalculator *_calculator = nullptr;
	MapTransferHandler *_mapHandler = nullptr;

	Pool<PointStore *> _emptyPool;
	std::map<std::string, Pool<PointStore *> > _loadedPool;

	std::vector<std::string> _elements;
	int _threads = 1;
	int _storeCount = 3;
};

#endif
