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

#ifndef __vagabond__AtomWarp__
#define __vagabond__AtomWarp__

#include <vagabond/utils/AcquireCoord.h>

class PositionalCluster;
class TorsionCluster;
class MetadataGroup;
class RAMovement;
class ClusterSVD;
class Instance;
class Atom;

class AtomWarp
{
public:
	AtomWarp(std::vector<Instance *> instances, Instance *reference);

	std::function<glm::vec3(const Coord::Get &get, int num)> 
	mappedMotions(int n, const std::vector<Atom *> &order);

	std::vector<Parameter *> 
	orderedParameters(const std::vector<Parameter *> &set, int n);

	std::function<float(Parameter *)>
	parameterMagnitudes(const std::vector<Parameter *> &set, int nAxes);

	MetadataGroup *const &torsionData() const
	{
		return _torsionData;
	}

	ClusterSVD *const &cluster() const
	{
		return _tCluster;
	}
private:
	std::vector<RAMovement> allMotions(int n);

	MetadataGroup *_torsionData = nullptr;
	ClusterSVD *_tCluster = nullptr;
	Instance *_reference = nullptr;
	
	int _lastN = 0;
};

#endif
