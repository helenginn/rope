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

#ifndef __vagabond__Warper__
#define __vagabond__Warper__

#include <vagabond/core/Responder.h>
#include <thread>
#include <set>

class Monitor;
class Entity;
class Path;

class Warper : public HasResponder<Responder<Warper *> >
{
public:
	Warper(Monitor *monitor);

	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	Entity *entity() const
	{
		return _entity;
	}
	
	void setMainCluster(TorsionCluster *cluster)
	{
		_mainCluster = cluster;
	}
	
	TorsionCluster *pathCluster()
	{
		return _pathCluster;
	}

	void setup();
private:
	static void doJobs(Warper *self);

	void run();
	void wait();

	TorsionCluster *torsionClusterForPathDeviations(int steps);

	Monitor *_monitor = nullptr;
	Entity *_entity = nullptr;

	TorsionCluster *_mainCluster = nullptr;
	TorsionCluster *_pathCluster = nullptr;
	
	std::set<Path *> _paths;
	std::thread *_worker = nullptr;
};

#endif
