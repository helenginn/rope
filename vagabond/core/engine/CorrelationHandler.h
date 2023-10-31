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
#include "engine/ElementTypes.h"
#include <fftw3.h>

class BondCalculator;
class MapSumHandler;

template<class T>
class OriginGrid;

class Correlator;
class AtomMap;

typedef std::pair<Correlator *, AtomMap *> CorrelMapPair;

class CorrelationHandler : public Handler
{
public:
	CorrelationHandler(BondCalculator *calculator);
	CorrelationHandler(OriginGrid<fftwf_complex> *reference,
	                   const AtomMap *calc_template, int resources);
	~CorrelationHandler();

	void get_correlation(Task<SegmentAddition, AtomMap *> *made_map,
	                     Task<CorrelMapPair, Correlation> **get_cc = nullptr);

	Correlator *acquireCorrelatorIfAvailable();
	void returnCorrelator(Correlator *cc);

	void setMapSumHandler(MapSumHandler *sumHandler)
	{
		_sumHandler = sumHandler;
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

	void finish();
private:
	void createCorrelators();

	BondCalculator *_calculator = nullptr;
	MapSumHandler *_sumHandler = nullptr;
	OriginGrid<fftwf_complex> *_refDensity = nullptr;
	const AtomMap *_template = nullptr;
	
	struct CorrelJob
	{
		Job *job;
		AtomMap *map;
	};
	
	Pool<Correlator *> _correlPool;
	Pool<CorrelJob *> _mapPool;

	int _threads = 1;
};

#endif
