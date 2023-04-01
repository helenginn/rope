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

#ifndef __vagabond__HiddenLayer__
#define __vagabond__HiddenLayer__

#include "CalcLayer.h"

class HiddenLayer : public CalcLayer
{
public:
	HiddenLayer();
	
	friend std::ostream &operator<<(std::ostream &ss, const HiddenLayer *l);

	virtual size_t requestedEntries();
protected:
	virtual void setup();
	virtual void connect();
	virtual void forwardTasks();
	virtual void backwardTasks();
private:
	VectorLoc _nextSensitivities = {nullptr, 0};
	MatrixLoc _nextWeights = {nullptr, 0, 0};
};

#endif
