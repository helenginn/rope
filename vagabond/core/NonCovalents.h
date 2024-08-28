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
#include <vagabond/utils/OpSet.h>
#include <vagabond/utils/OpVec.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/glm_import.h>

class Atom;
class Instance;
class BondSequence;
template <class X, class Y> class Task;

class NonCovalents
{
public:
	NonCovalents();

	void addInstance(Instance *a)
	{
		if (_instances.size() == 0)
		{
			_invariant = a;
		}

		if (std::find(_instances.begin(), _instances.end(), a) ==
		    _instances.end())
		{
			if (_instances.size() > 0)
			{
				_instance2Idx[a] = _instances.size() - 1;
			}
			_instances.push_back(a);
		}
	}
	
	const bool &ready() const
	{
		return _ready;
	}
	
	void provideSequence(BondSequence *const &seq)
	{
		prepare(seq);
		_ready = true;
	}

	std::function<BondSequence *(BondSequence *)> align_task(const float &frac);

	struct WeightedSum
	{
		WeightedSum(Atom *atom, const std::vector<Atom *> &fiducials);

		Atom *atom;
		std::vector<Atom *> fiducials;
		float ave_weight = 1;

		typedef std::function<glm::vec3(Atom *)> GetPos;
		
		OpVec<float> weights_for_positions(const GetPos &getPos);

		glm::vec3 position_for_weights(const GetPos &getPos,
		                               const OpVec<float> &weights);

		std::function<OpVec<float>(float)> weights_for_frac;

		std::function<Eigen::VectorXf(float)> weights_to_matrix_column;
	};

	struct Interface
	{
		Instance *left{}, *right{};
		
		struct Side
		{
			OpSet<Atom *> atoms; // pointers to atoms
			std::vector<int> seq_idxs; // pointers to corresponding 
									   // AtomBlock in BondSequence
			std::map<Atom *, int> locs; // atom to seq_idxs index
			
			void reindex();
		};

		Side lefts, rights;
		std::vector<WeightedSum> sums;
	};

private:
	void prepare(BondSequence *const &seq);
	void prepareMatrix();

	struct MatId
	{
		int row; // instance, should already be multiplied by 4
		int col; // column corresponding to atom info
		int idx; // index of BondSequence's AtomBlocks
		float weight = 1; // per-sum weight
	};

	std::vector<MatId> 
	matrix_coordinates(const OpSet<Atom *> &all,
	                   const std::function<int(Atom *const &)> 
	                   &atom_idx);

	std::vector<MatId> target_coordinates();
	void prepareTargets();

	void prepareCoordinateColumns(const std::function<int(Atom *const &)> &atom_idx);

	void assignInstancesToAtoms(BondSequence *const &seq);
	void prepareBarycentricWeights();
	void preparePositionMatrix();
	void prepareBarycentricTargetMatrices();

	void findInterfaces(const std::function<int(Atom *const &)> &func);
	NonCovalents::Interface findInterface(Instance *first, Instance *second);

	std::function<BondSequence *(BondSequence *)> align(const float &frac);

	std::vector<Instance *> _instances;
	std::map<Instance *, int> _instance2Idx;
	std::vector<Interface> _faces;
	std::function<void(BondSequence *seq, Eigen::MatrixXf &dest,
	                   bool trans_only)> _blocksToMatrixPositions;
	std::function<void(BondSequence *seq, 
	                   Eigen::MatrixXf &dest)> _blocksToTargetMatrix;
	std::function<void(const float &frac,
	                   Eigen::MatrixXf &dest)> _weightsToMatrixPositions;

	std::map<int, MatId> _seqToId;
	std::vector<MatId> _matIds;

	std::map<Instance *, std::vector<int>> _atomNumbers;

	Eigen::MatrixXf _positions;
	Eigen::MatrixXf _barycentrics;
	Eigen::MatrixXf _targets;

	Eigen::MatrixXf _leftMatrix;
	Eigen::MatrixXf _rightMatrix;
	
	Instance *_invariant = nullptr;
	bool _ready = false;
};

#endif
