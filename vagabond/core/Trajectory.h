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

#ifndef __vagabond__Trajectory__
#define __vagabond__Trajectory__

#include "RTAngles.h"

class MetadataGroup;
class Path;
class RTMultiple;

class Trajectory
{
public:
	Trajectory(const RTMultiple &angles, Path *origin);
	~Trajectory();
	
	size_t size() const;
	
	RTAngles nakedTrajectory(int idx) const;

	void subtract(Trajectory *other);
	
	void attachInstance(Instance *inst);
	
	void addToMetadataGroup(MetadataGroup &group);
	
	float angleForFraction(float frac, int idx);
	RTAngles anglesForFraction(float frac);
	void relativeToFirst();

	void filterAngles(const std::vector<Parameter *> &ps);
private:
	void calculateLengths();
	
	int priorStep(float frac);

	RTMultiple *_traj = nullptr;
	std::vector<float> _steps;
	Path *_path = nullptr;

	float _step = 1;
};

#endif
