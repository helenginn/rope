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

#ifndef __vagabond__NonCovalents__
#define __vagabond__NonCovalents__

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/glm_import.h>

class Instance;
class BondSequence;
template <class X, class Y> class Task;

class NonCovalents
{
public:
	NonCovalents();

	void addBond(Instance *a, Instance *b, const std::string &left,
	             const std::string &right)
	{
		_bonds.push_back({a, b, left, right});
		if (std::find(_instances.begin(), _instances.end(), a) ==
		    _instances.end())
		{
			_instances.push_back(a);
		}
		if (std::find(_instances.begin(), _instances.end(), b) ==
		    _instances.end())
		{
			_instances.push_back(b);
		}
	}
	
	void provideSequence(BondSequence *const &seq)
	{
		prepare(seq);
	}

	std::function<BondSequence *(BondSequence *)> align_task();
private:
	void prepare(BondSequence *const &seq);
	void prepareMatrix();
	void prepareOne();
	
	struct Bond
	{
		Instance *left_instance{};
		Instance *right_instance{};
		std::string left{};
		std::string right{};
		int left_lookup = -1; 
		int right_lookup = -1;
		int middle_lookup = -1;
		bool left_is_donor = false;
		float frac = 0.;
		
		std::function<glm::vec3(BondSequence *)> left_contributor;
		std::function<glm::vec3(BondSequence *)> right_contributor;

		int left_drop[2] = {-1, -1};
		int right_drop[2] = {-1, -1};
	};

	void findBondedAtoms(BondSequence *const &seq, Bond &bond);
	void findMiddleAtoms(BondSequence *const &seq, Bond &bond);
	void assignInstancesToAtoms(BondSequence *const &seq);

	std::function<BondSequence *(BondSequence *)> align();

	std::vector<Instance *> _instances;
	std::vector<Bond> _bonds;
	std::map<Instance *, std::map<Instance *, Bond *>> _map;
	std::map<Instance *, std::vector<int>> _atomNumbers;

	Eigen::MatrixXf _leftMatrix;
	Eigen::MatrixXf _rightMatrix;
};

#endif
