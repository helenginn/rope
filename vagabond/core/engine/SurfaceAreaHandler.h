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

#ifndef __vagabond__SurfaceAreaHandler__
#define __vagabond__SurfaceAreaHandler__

#include "Handler.h"

class AreaMeasurer;
class BondCalculator;

class SurfaceAreaHandler : public Handler
{
public:
	SurfaceAreaHandler(BondCalculator *calculator);
	~SurfaceAreaHandler();

	void setThreads(const int threads)
	{
		_threads = threads;
	}

	void setMeasurerCount(const size_t measureNum)
	{
		_measureNum = measureNum;
	}

	/** after atom position calculation, register job for area calculation */
	void sendJobForCalculation(Job *job, AtomPosMap &aps);

	/** call after calculation of surface area to return to the idle pool */
	void returnToIdle(AreaMeasurer *am);

	/** get an area measurer which has atom positions pre-loaded to calculate */
	AreaMeasurer *acquireMeasurerToCalculate();
	
	/** set up resources which are needed for calculations */
	void setup();

	/** set up workers and corresponding threads, begin calculations */
	void start();

	/** stop all work, join up threads and delete threads/workers */
	void finish();
	
	BondCalculator *calculator() const
	{
		return _calculator;
	}
private:
	void prepareThreads();
	void createMeasurers();

	size_t _measureNum = 1;
	int _threads = 1;

	Pool<AreaMeasurer *> _areaPool;
	Pool<AreaMeasurer *> _idlePool;

	BondCalculator *_calculator = nullptr;
	std::vector<AreaMeasurer *> _toDelete;
};

#endif
