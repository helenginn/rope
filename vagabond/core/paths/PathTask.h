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

#ifndef __vagabond__PathTask__
#define __vagabond__PathTask__

#include "../Item.h"
#include <atomic>

class PlausibleRoute;
class PathFinder;
class HasMetadata;
class Instance;
class Model;

class PathTask : public Item
{
public:
	PathTask(PathFinder *pf);

	bool tryLock();
	void unlockAll();

	bool tryLockModel(Model *m);
	
	virtual void run();
	virtual bool runnable()
	{
		return false;
	}

	enum TaskType
	{
		None,
		Reporter,
		Validation,
		Optimisation,
	};
	
	virtual TaskType type()
	{
		return None;
	}
	
	bool complete() const
	{
		return _complete;
	}

	static Model *modelForHasMetadata(HasMetadata *wanted);
	void gatherTasks(std::vector<PathTask *> &collection);
	
	PathTask *task(int i) const;
protected:
	virtual void specificTasks() {};

	std::set<Model *> _needs;
	std::set<Model *> _locks;
	PathFinder *_pf = nullptr;
	
	void setComplete();
private:
	std::atomic<bool> _complete{false};
};

#endif


