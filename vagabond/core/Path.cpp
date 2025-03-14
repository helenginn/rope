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
#include "PathData.h"
#include "ModelManager.h"
#include "PathManager.h"
#include "RTMultiple.h"

Path::Path(PlausibleRoute *pr)
{
	_route = pr;
	pr->refreshScores();
	acquireSingleProperties(_route);

	_startInstance = pr->instance()->id();
	_endInstance = pr->endInstance()->id();
	_model_id = pr->instance()->model()->id();
	_instance = pr->instance();
	_end = pr->endInstance();

	_motions = pr->motions();
	
	_route = nullptr;
}

Path::~Path()
{
	
}

const Metadata::KeyValues Path::metadata(Metadata *source) const
{
	Metadata::KeyValues chosen;
	if (source == nullptr) return chosen;

	const Metadata::KeyValues *ptr = source->valuesForInstance(id());
	if (!ptr) return chosen;

	return *ptr;
}

void Path::signalDeletion()
{
	sendResponse("purge", this);
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

void Path::acquireSingleProperties(Route *route)
{
	route->setHash();
	_momentum = route->momentumScore();
	_activationEnergy = route->activationEnergy();
	_torsionEnergy = route->torsionEnergy();
	_clash = route->clashScore();
	_maxMomentumDistance = route->maximumMomentumDistance();
	_maxClashDistance = route->maximumClashDistance();
	_hash = route->hash();
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

	PlausibleRoute *pr = new PlausibleRoute(_instance, _end);
	pr->setNew(false);
	pr->setMotions(_motions);
	pr->setScores(_momentum, _clash);
	pr->setMaximumMomentumDistance(_maxMomentumDistance);
	pr->setTorsionEnergy(_torsionEnergy);
	pr->setMaximumClashDistance(_maxClashDistance);
	pr->setActivationEnergy(_activationEnergy);
	pr->setHash(_hash);
	
	_route = pr;
	
	return pr;
}

std::string Path::desc() const
{
	return _startInstance + " to " + _endInstance;
}

void Path::cleanupRoute()
{
	if (_route)
	{
		acquireSingleProperties(_route);

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

void Path::addToData(PathData *pd)
{
	std::vector<DataFloat> entry(pd->length());
	DataFloat *pos = &entry[0];

	for (size_t i = 0; i < _motions.size(); i++)
	{
		Motion &motion = _motions.storage(i);
		if (!_motions.rt(i).torsion().coversMainChain())
		{
//			continue;
		}

		motion.writeToData(pos);
	}

	pd->addMetadataArray(this, entry);
}
