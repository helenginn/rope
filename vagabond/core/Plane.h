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

#ifndef __vagabond__Plane__
#define __vagabond__Plane__

#include <vagabond/c4x/Cluster.h>
#include "MetadataGroup.h"
#include "StructureModification.h"
#include "Responder.h"

class Molecule;
class ForceField;

class Plane : public StructureModification, public HasResponder<Responder<Plane> >
{
public:
	Plane(Molecule *mol, Cluster<MetadataGroup> *cluster);

	void addAxis(std::vector<ResidueTorsion> &list, std::vector<float> &values,
	             std::vector<float> &mapped);
	
	const float &score(int idx)
	{
		return _scores[idx];
	}
	
	const float &reference()
	{
		return _scores[_refIdx];
	}
	
	const float &mean()
	{
		return _mean;
	}
	
	const float &stdev()
	{
		return _stdev;
	}

	int toIndex(int x, int y) const
	{
		x -= _counts[0];
		y -= _counts[1];
		if (_toIndex.count(x) && _toIndex.at(x).count(y))
		{
			return _toIndex.at(x).at(y);
		}
		else
		{
			return -1;
		}
	}
	
	void setCount(int idx, int count)
	{
		_counts[idx] = count;
	}
	
	int num(int idx)
	{
		return count(idx) * 2;
	}
	
	int count(int idx)
	{
		return _counts[idx];
	}
	
	void setScale(float scale)
	{
		_scale = scale;
	}
	
	void refresh();
	void setupForceField();
	void reportScores();

	glm::vec3 generateVertex(int i, int j);
private:
	void submitJob(float x, float y);
	void collectResults();

	std::vector<glm::vec3> _points;
	std::map<int, std::map<int, int> > _toIndex;

	std::vector<float> _scores;
	std::map<int, int> _scoreMap;

	std::vector<float> _torsions[2];
	std::vector<float> _axes[2];
	int _counts[2] = {20, 20};
	float _scale = 0.5;
	
	float _mean;
	float _stdev;
	int _refIdx;

	ForceField *_forceField = nullptr;
	Cluster<MetadataGroup> *_cluster = nullptr;
};

#endif