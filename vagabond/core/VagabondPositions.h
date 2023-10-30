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

#ifndef __vagabond__VagabondPositions__
#define __vagabond__VagabondPositions__

#include "StructureModification.h"
#include "Engine.h"

class VagabondPositions : public StructureModification, public RunsEngine
{
public:
	VagabondPositions(AtomGroup *const &group);

	void setup();

	float fullResidual();

	bool refineBetween(int min, int max);

	size_t totalPositions();
protected:
	typedef std::function<void(std::vector<float> &values)> Getter;
	typedef std::function<void(const std::vector<float> &values)> Setter;

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);

	void getSetCoefficients(const std::set<Parameter *> &params,
	                        Getter &getter, Setter &setter);

	int submitJob();
	Result *submitJobAndRetrieve();
	virtual void prepareResources();
private:
	AtomGroup *const &_group;

	std::vector<float> _parameters;
	int _ticket = 0;
	Getter _getter{};
	Setter _setter{};
};

#endif
