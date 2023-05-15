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

#include "MetadataGroup.h"
#include "RopeCluster.h"
#include "Monitor.h"
#include "Entity.h"
#include "Warper.h"
#include "Path.h"

Warper::Warper(Monitor *monitor)
{
	_monitor = monitor;

}

void Warper::wait()
{
	if (_worker)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}
}

void Warper::setup()
{
	_paths = _monitor->paths();

	_worker = new std::thread(Warper::doJobs, this);
}

void Warper::doJobs(Warper *self)
{
	self->run();
}

void Warper::run()
{
	_pathCluster = torsionClusterForPathDeviations(4);
	_pathCluster->cluster();

	if (_pathCluster->objectGroup()->objectCount() > 0)
	{
		_pathCluster->calculateInverse();
		sendResponse("warper_star", nullptr);
	}
	
}

TorsionCluster *Warper::torsionClusterForPathDeviations(int steps)
{
	MetadataGroup angles = _entity->makeTorsionDataGroup(true);

	for (Path *path : _paths)
	{
		path->setStepCount(steps);
		path->addDeviationsToGroup(angles);
	}

	TorsionCluster *cluster = new TorsionCluster(angles);
	cluster->setSubtractAverage(false);
	std::cout << "Objects: " << cluster->objectGroup()->objectCount() << std::endl;
	return cluster;
}
