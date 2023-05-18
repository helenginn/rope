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

#include "Path.h"
#include "ModelManager.h"
#include "PathManager.h"
#include "Trajectory.h"
#include "RTMultiple.h"

Path::Path(PlausibleRoute *pr)
{
	_route = pr;
	_startInstance = pr->instance()->id();
	_endInstance = pr->endInstance()->id();
	_model_id = pr->instance()->model()->id();
	_instance = pr->instance();
	_end = pr->endInstance();
	_type = pr->type();

	_motions = pr->motions();
	
	_route = nullptr;
}

Path::~Path()
{
	
}

void Path::housekeeping()
{
	if (_model && _instance && _end)
	{
		return;
	}

	_model = Environment::env().modelManager()->model(_model_id);

	if (!_model)
	{
		return;
	}
	
	for (Instance &inst : _model->polymers())
	{
		if (inst.id() == _startInstance)
		{
			_instance = &inst;
		}
		
	}
	
	for (Model &m : Environment::env().modelManager()->objects())
	{
		for (Instance &inst : m.polymers())
		{
			if (inst.id() == _endInstance)
			{
				_end = &inst;
			}
		}
	}
	
	_instance->setResponder(this);
	_end->setResponder(this);
}

PlausibleRoute *Path::toRoute()
{
	housekeeping();
	
	if (!_instance || !_model)
	{
		throw std::runtime_error("Unable to find model/instance in environment.");
	}
	
	_model->currentAtoms();

	_motions.attachInstance(_instance);

	PlausibleRoute *pr = new PlausibleRoute(_instance, nullptr, _motions.size());
	pr->useForceField(false);
	pr->setDestinationInstance(_end);
	pr->setType(_type);
	pr->setMotions(_motions);

	_route = pr;
	
	return pr;
}

std::string Path::desc() const
{
	return _startInstance + " to " + _endInstance;
}

Trajectory *Path::calculateDeviations(int steps)
{
	Trajectory *straight = calculateTrajectory(2);
	Trajectory *curvy = calculateTrajectory(steps);
	
	curvy->subtract(straight);
	return curvy;
}

Trajectory *Path::calculateTrajectory(int steps)
{
	housekeeping();

	_instance->load();
	AtomContent *grp = _instance->model()->currentAtoms();
	PlausibleRoute *pr = toRoute();
	pr->setup();
	
	pr->calculatePolynomialProgression(steps);
	
	std::vector<ResidueTorsion> polymer_rts = _instance->residueTorsionList();
	std::cout << "instance list: " << polymer_rts.size() << std::endl;
	RTMultiple list; list.vector_from(polymer_rts);
	RTAngles angles; angles.vector_from(polymer_rts);
	RTAngles first;
	
	for (size_t i = 0; i < steps; i++)
	{
		pr->submitJobAndRetrieve(i);
		_instance->extractTorsionAngles(grp, true);
		_instance->grabTorsions(angles, rope::TemporaryTorsions);
		
		if (i == 0)
		{
			first = angles;
		}

		angles.match_degrees_to(first);
		
		first = angles;

		list.add_angles_from(angles);
	}
	
	_instance->unload();
	cleanupRoute();
	
	Trajectory *traj = new Trajectory(list, this);
	return traj;
}

void Path::addDeviationsToGroup(MetadataGroup &group)
{
	Trajectory *traj = calculateDeviations(_steps);
	traj->addToMetadataGroup(group);
	delete traj;
}

void Path::addTorsionsToGroup(MetadataGroup &group)
{
	if (!_contributeSVD)
	{
		return;
	}

	Trajectory *traj = calculateTrajectory(_steps);
	traj->addToMetadataGroup(group);
	delete traj;
}

void Path::cleanupRoute()
{
	if (_route)
	{
		delete _route;
		_route = nullptr;
	}
}

bool Path::operator==(const Path &other) const
{
	if (_startInstance == other._startInstance &&
	    _endInstance == other._endInstance)
	{
		return true;
	}

	return false;
}

bool Path::sameRouteAsPath(Path *other)
{
	return (startInstance() == other->startInstance() &&
	        endInstance() == other->endInstance());
}

void Path::sendObject(std::string tag, void *object)
{
	if (tag == "purge")
	{
		Environment::pathManager()->purgePath(this);
	}

}

