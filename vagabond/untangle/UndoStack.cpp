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

#include <iostream>
#include "UndoStack.h"

UndoStack::UndoStack()
{

}


void UndoStack::addJobAndExecute(std::function<void()> forward,
                                 std::function<void()> reverse)
{
	// if we're behind in the undo stack, delete all jobs in the old branch
	if (_tracker < 0)
	{
		_jobs.erase(_jobs.end() + _tracker, _jobs.end());
		_tracker = 0;
	}

	_jobs.push_back({forward, reverse});
	forward();
}

void UndoStack::undo()
{
	if (_jobs.size() + _tracker <= 0)
	{
		// end of stack
		return;
	}

	auto it = _jobs.end() + _tracker;
	it--;
	it->reverse();
	_tracker--;
}

void UndoStack::redo()
{
	if (_tracker == 0)
	{
		return;
	}

	auto it = _jobs.end() + _tracker;
	it->forward();
	_tracker++;
}
