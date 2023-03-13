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

Path::Path(PlausibleRoute *pr)
{
	_route = pr;
	_startInstance = pr->instance()->id();
	_endInstance = pr->endInstance()->id();
	_model_id = pr->instance()->model()->id();
	_instance = pr->instance();
	_end = pr->endInstance();
	_rts = pr->residueTorsions();

	_wayPoints = pr->wayPoints();
	_flips = pr->flips();
	_type = pr->type();
	_destination = pr->destination();
	
	for (size_t i = 0; i < pr->Route::parameterCount(); i++)
	{
		getTorsionRef(i);
	}
	
	_route = nullptr;
}

void Path::getTorsionRef(int idx)
{
	Parameter *t = _route->parameter(idx);

	ResidueId id = t->residueId();
	TorsionRef rt = TorsionRef(t);
	rt.setRefinedAngle(_destination[idx]);
}

void Path::housekeeping()
{
	_destination.clear();

	for (ResidueTorsion &rt : _rts)
	{
		_destination.push_back(rt.torsion().refinedAngle());
	}

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
}

PlausibleRoute *Path::toRoute()
{
	housekeeping();
	
	if (!_instance || !_model)
	{
		throw std::runtime_error("Unable to find model/instance in environment.");
	}
	
	_model->load();
	AtomGroup *group = _model->currentAtoms();

	SplitRoute *pr = new SplitRoute(_instance, nullptr, _wayPoints.size());
	pr->setDestinationInstance(_end);
	pr->setType(_type);
	pr->setDestination(_destination);
	pr->setWayPoints(_wayPoints);
	pr->Route::setFlips(_flips);

	for (size_t i = 0; i < _rts.size(); i++)
	{
		_rts[i].attachToInstance(_instance);
		Parameter *p = _rts[i].parameter();
		
		if (p == nullptr)
		{
			std::cout << "WARNING! null parameter in " << 
			_rts[i].local_id().str() << std::endl;
		}
		
		pr->addParameter(_rts[i], p);
	}

	_route = pr;
	
	return pr;
}

std::string Path::desc() const
{
	return _startInstance + " to " + _endInstance;
}

void Path::calculateArrays(MetadataGroup *group)
{
	const int total = 12;
	housekeeping();

	_instance->load();
	AtomContent *grp = _instance->model()->currentAtoms();
	PlausibleRoute *pr = toRoute();
	pr->setup();
	pr->calculateProgression(total);
	
	for (size_t i = 0; i < total; i++)
	{
		pr->submitJobAndRetrieve(i);
		_instance->extractTorsionAngles(grp, true);
		MetadataGroup::Array vals;
		vals = _instance->grabTorsions(rope::TemporaryTorsions);
		group->matchDegrees(vals);
		_angleArrays.push_back(vals);
	}
	
	_instance->unload();
}

void Path::addTorsionsToGroup(MetadataGroup &group)
{
	if (_angleArrays.size() == 0)
	{
		calculateArrays(&group);
	}
	
	if (_contributeSVD)
	{
		std::cout << "here!" << std::endl;
		for (size_t i = 0; i < _angleArrays.size(); i++)
		{
			group.addMetadataArray(this, _angleArrays[i]);
		}
	}

}

void Path::cleanupRoute()
{
	if (_route)
	{
		delete _route;
		_route = nullptr;
	}
}
