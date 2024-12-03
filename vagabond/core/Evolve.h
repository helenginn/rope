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

#ifndef __vagabond__Evolve__
#define __vagabond__Evolve__

#include "Separation.h"
#include <vagabond/core/StructureModification.h>
#include <vagabond/utils/Eigen/Dense>
#include <thread>

class Instance;

class Evolve : public StructureModification
{
public:
	Evolve(Instance *mol);
	
	void setResidueRange(int min, int max) // inclusive
	{
		_minRes = min;
		_maxRes = max;
	}
	
	~Evolve();

	void setup();
private:
	struct CloseContact
	{
		Atom *f;
		Atom *g;
		float dist;
	};

	void supplyTorsions(CoordManager *manager);
	void submitJobAndRetrieve();
	Eigen::MatrixXf divergence();
	Eigen::MatrixXf convergence();
	void closeContacts();
	void evolve();

	void prepareResources();
	glm::vec3 reference_axis(int idx);
	std::set<int> findIndices();
	Instance *_instance;
	
	int _min = 0;
	int _max = 0;
	
	int _minRes = 0;
	int _maxRes = 0;

	float _correction = 0.1f;
	float _stabilisation = 0.00f;
	int _num = 0;
	std::thread *_worker = nullptr;

	std::vector<float> _angles;
	std::map<int, float> _mods;
	
	std::vector<CloseContact> _contacts;
	Separation _sep;
};

#endif
