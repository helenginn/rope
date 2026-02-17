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

#ifndef __vagabond__MultiEngine__
#define __vagabond__MultiEngine__

#include <engine/Task.h>

class Engine;
class Tasks;
class BaseTask;

class MultiEngine
{
public:
	void addImmediateTask(BaseTask *bt);
	void addHangingTask(BaseTask *bt);
	
	Tasks *const &immediate() const
	{
		return _immediate;
	}

	Tasks *const &hanging() const
	{
		return _hanging;
	}
	
	void clearTasks();

	virtual void declareDone(Engine *sender, const std::vector<float> &best) = 0;
protected:
	MultiEngine();
	~MultiEngine();
	
	Tasks *_immediate = nullptr;
	Tasks *_hanging = nullptr;
	int _threads = 4;

private:

};

#endif
