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

#ifndef __vagabond__BondCalculator__
#define __vagabond__BondCalculator__

#include "engine/Handler.h"

template <typename I, typename O> class Task;

/** \class BondCalculator
 *  Used to be master class for the internal Vagabond engine.
 *  Now just a repository for finished results. Accompanying image is out of date.
 *
 * \image html vagabond/doc/bondcalculator_class_connections.png width=1200px
 */

class BondCalculator : public Handler
{
public:
	BondCalculator();
	~BondCalculator();
	
	void submitResult(Result *result);
	void submitResult(const Result &r);
	
	void holdHorses();
	void releaseHorses();
	
	Task<Result, void *> *submitResult(int ticket);
	
	Result *emptyResult();
	Result *acquireResult();
private:
	ExpectantPool<Result *> _resultPool;
};

#endif
