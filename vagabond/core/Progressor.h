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

#ifndef __vagabond__Progressor__
#define __vagabond__Progressor__

#include "Environment.h"
#include <iostream>

/** \class Progressor Subclasses of Progressor can be hooked to a 
 * ProgressorResponder. This allows a GUI to report some kind of
 * progress bar/statistics to the user while something is loading, which
 * cannot or should not be interrupted */

class Progressor;

class ProgressResponder
{
public:
	virtual ~ProgressResponder() {};
	virtual void clickTicker(Progressor *progressor) = 0;
	virtual void finish() {};
};

class Progressor
{
public:
	Progressor()
	{
	}

	virtual ~Progressor() {};
	
	void setProgressResponder(ProgressResponder *responder)
	{
		_responder = responder;
		_ticks = 0;
	}
	
	virtual const std::string progressName() const = 0;

	void clickTicker()
	{
		_ticks++;
		
		if (!_responder)
		{
			setProgressResponder(Environment::env().progressResponder());
		}

		if (_responder)
		{
			_responder->clickTicker(this);
		}
	}
	
	const int &ticks() const
	{
		return _ticks;
	}
protected:
	ProgressResponder *_responder = nullptr;

	int _ticks = 0;
};

#endif
