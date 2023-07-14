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
#include "Trajectory.h"
#include "MetadataGroup.h"
#include "RTMultiple.h"

Trajectory::Trajectory(const RTMultiple &angles, Path *origin)
{
	_traj = new RTMultiple(angles);
	calculateLengths();
	_path = origin;
}

Trajectory::~Trajectory()
{
	delete _traj;
}

void Trajectory::relativeToFirst()
{
	if (_traj->steps() <= 1)
	{
		return;
	}

	RTAngles sample = _traj->angles_for_index(0);
	for (int i = 0; i < _traj->steps(); i++)
	{
		_traj->subtract_angles(sample, i);
	}
}

void Trajectory::filterAngles(const std::vector<Parameter *> &ps)
{
	int steps = _traj->steps();
	_traj->filter_according_to(ps);
	_traj->ensure_steps(steps);
}

void Trajectory::calculateLengths()
{
	float total = _traj->steps();
	_steps.reserve(total);

	_step = 1 / (total - 1);

	for (int i = 0; i < _traj->steps(); i++)
	{
		_steps.push_back((float)i * _step);
	}
}

float Trajectory::angleForFraction(float frac, int idx)
{
	if (frac < 0) { frac = 0; }
	if (frac > 1) { frac = 1; }

	int n = priorStep(frac);
	
	if (n < 0)
	{
		return 0;
	}
	
	float tot = frac - _steps[n];
	tot /= _step;
	
	float angle = _traj->angle_for_index(n, idx);
	float next = _traj->angle_for_index(n + 1, idx);

	float diff = next - angle;
	diff *= tot;
	float ret = angle + diff;

	return ret;
}

RTAngles Trajectory::anglesForFraction(float frac)
{
	RTAngles ret; ret.vector_from(_traj->headers_only());

	if (frac < 0 || frac > 1)
	{
		return ret;
	}

	int n = priorStep(frac);
	
	if (n < 0)
	{
		return ret;
	}
	
	float tot = frac - _steps[n];
	tot /= _step;

	RTAngles angles = _traj->angles_for_index(n);
	RTAngles next = _traj->angles_for_index(n + 1);

	for (int i = 0; i < angles.size(); i++)
	{
		float diff = next.storage(i) - angles.storage(i);
		diff *= tot;
		ret.storage(i) = angles.storage(i) + diff;
	}
	
	return ret;
}

void Trajectory::subtract(Trajectory *other)
{
	for (int i = 0; i < _traj->steps(); i++)
	{
		float frac = _steps[i];
		RTAngles sample = other->anglesForFraction(frac);
		_traj->subtract_angles(sample, i);
	}
}

RTAngles Trajectory::nakedTrajectory(int idx) const
{
	return _traj->angles_for_index(idx);
}

void Trajectory::addToMetadataGroup(MetadataGroup &group)
{
	for (size_t i = 0; i < size(); i++)
	{
		RTAngles angles = nakedTrajectory(i);
		group.addMetadataArray(_path, angles);
	}
}

size_t Trajectory::size() const
{
	return _traj->steps();
}

void Trajectory::attachInstance(Instance *inst)
{
	_traj->attachInstance(inst);
}

std::vector<Angular> Trajectory::anglesForIndex(int idx)
{
	RTAngles sample = _traj->angles_for_index(idx);
	std::vector<Angular> fs = sample.storage_only();
	return fs;
}
