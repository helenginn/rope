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

#ifndef __vagabond__CorrelationHandler__
#define __vagabond__CorrelationHandler__

#include <thread>
#include "engine/Handler.h"

class BondCalculator;
class MapSumHandler;

template<class T>
class OriginGrid;

class AtomSegment;
class Correlator;

class CorrelationHandler : public Handler
{
public:
	CorrelationHandler(BondCalculator *calculator);
	~CorrelationHandler();

	AtomSegment *acquireMap(Job **job);
	Correlator *acquireCorrelator();
	void pushMap(AtomSegment *seg, Job *job);
	void returnCorrelator(Correlator *cc);

	void setMapSumHandler(MapSumHandler *sumHandler)
	{
		_sumHandler = sumHandler;
	}

	void setThreads(int threads)
	{
		_threads = threads;
	}
	
	void setReferenceDensity(OriginGrid<fftwf_complex> *dens)
	{
		_refDensity = dens;
	}
	
	BondCalculator *calculator()
	{
		return _calculator;
	}

	void setup();

	void start();
	void finish();
	void joinThreads();
private:
	void signalThreads();
	void prepareThreads();
	void createCorrelators();

	BondCalculator *_calculator = nullptr;
	MapSumHandler *_sumHandler = nullptr;
	OriginGrid<fftwf_complex> *_refDensity = nullptr;
	
	struct CorrelJob
	{
		Job *job;
		AtomSegment *map;
	};
	
	Pool<Correlator *> _correlPool;
	Pool<CorrelJob *> _mapPool;

	int _threads = 1;
};

#endif
