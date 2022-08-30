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

#ifndef __vagabond__ForceFieldHandler__
#define __vagabond__ForceFieldHandler__

#include "Handler.h"
#include "FFProperties.h"

class BondCalculator;
class ForceField;

class ForceFieldHandler : public Handler
{
public:
	ForceFieldHandler(BondCalculator *calculator = nullptr);
	~ForceFieldHandler();

	void atomMapToForceField(Job *j, AtomPosMap &aps);
	
	void setFFProperties(FFProperties &props)
	{
		_props = props;
	}
	
	void setForceFieldCount(int num)
	{
		_ffCount = num;
	}
	
	void setThreads(int threads)
	{
		_threads = threads;
	}
	
	ForceField *acquireForceFieldToCalculate();
	void returnEmptyForceField(ForceField *ff);

	void setup();

	void start();
	void finish();
	void joinThreads();
	
	BondCalculator *calculator() const
	{
		return _calculator;
	}
private:
	void prepareThreads();
	void signalThreads();

	void createForceFields();
	void fillDefaults();

	BondCalculator *_calculator = nullptr;
	Pool<ForceField *> _emptyPool;
	Pool<ForceField *> _calculatePool;
	
	FFProperties _props{};

	int _ffCount = 1;
	int _threads = 1;
};

#endif
