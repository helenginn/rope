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
#include <utility>
#include <vagabond/utils/OpSet.h>
#include <vagabond/utils/OpVec.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/glm_import.h>
#include "AtomGroup.h"
#include "paths/Scores.h"

class Atom;
class Instance;
class BondSequence;
template <class X, class Y> class Task;

struct Segment
{
	Segment(int n)
	{
		num = n;
	}

	int num = -1;
	AtomGroup *grp = new AtomGroup();

	bool operator!=(const Segment &other) const
	{
		return !(*this == other);
	}
	
	
	bool operator==(const Segment &other) const
	{
		return (grp == other.grp && num == other.num);
	}
	
	bool operator<(const Segment &other) const
	{
		if (grp != other.grp)
		{
			return grp < other.grp;
		}
		
		return num < other.num;
	}
};

class NonCovalents
{
public:
	typedef std::pair<Atom *, Atom *> AtomPair;

	NonCovalents();

	void addInstance(Instance *a)
	{
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

	const std::vector<AtomPair> &connectionPairs() const
	{
		return _connectionPairs;
	}

	const std::vector<AtomPair> &fiducialConnectionPairs() const
	{
		return _fiducialConnectionPairs;
	}

	struct WeightedSum
	{
		WeightedSum(Atom *atom, const std::vector<Atom *> &fiducials);

		Atom *atom;
		std::vector<Atom *> fiducials;
		float ave_weight = 1;
		float weight_variance = 0;

		//std::vector<float> fiducial_variances;
		OpVec<float> start_weight_cache;

		typedef std::function<glm::vec3(Atom *)> GetPos;
		
		OpVec<float> weights_for_positions(const GetPos &getPos);

		glm::vec3 position_for_weights(const GetPos &getPos,
		                               const OpVec<float> &weights);

		// glm::vec3 average_fiducials_position();

		std::function<OpVec<float>(float)> weights_for_frac;

		std::function<Eigen::VectorXf(float)> weights_to_matrix_column;
	};

	struct Interface
	{
		Segment left{-1}, right{-1};
		
		struct Side
		{
			OpSet<Atom *> atoms; // pointers to atoms
			std::vector<int> seq_idxs; // pointers to corresponding 
									   // AtomBlock in BondSequence
			std::map<Atom *, int> locs; // atom to seq_idxs index
			
			void reindex();
		};

		Side lefts, rights;
		std::vector<AtomPair> atom_pairs;
		std::vector<WeightedSum> sums;
	};

	std::set<ScoreBucket> buckets();
private:
	void prepare(BondSequence *const &seq);
	void prepareMatrix();

	struct MatId
	{
		int row; // instance, should already be multiplied by 4
		int col; // column corresponding to atom info
		int idx; // index of BondSequence's AtomBlocks
		bool fixed; // is it invariant
	};

	std::vector<MatId> 
	matrix_coordinates(const OpSet<Atom *> &all,
	                   const std::function<int(Atom *const &)> 
	                   &atom_idx);

	std::vector<MatId> target_coordinates();
	void prepareTargets(const std::function<int(Atom *const &)> &func);

	void prepareCoordinateColumns(const std::function<int(Atom *const &)> &atom_idx);

	void assignSegmentsToAtoms(BondSequence *const &seq);
	void prepareBarycentricWeights();
	void preparePositionMatrix();
	void prepareBarycentricTargetMatrices();

	void findInterfaces(const std::function<int(Atom *const &)> &func);
	NonCovalents::Interface findInterface(Segment first, Segment second);

	std::function<BondSequence *(BondSequence *)> align(const float &frac);

	std::function<void(BondSequence *seq, Eigen::MatrixXf &dest,
	                   bool trans_only)> _blocksToMatrixPositions;
	std::function<void(const float &frac,
	                   Eigen::MatrixXf &dest)> _weightsToMatrixPositions;

	std::function<void(BondSequence *seq, 
	                   Eigen::MatrixXf &dest)> _snapToTargetColumns;

	std::map<Segment, std::vector<int>> _atomNumbers;

	std::vector<Segment> _segments;
	std::vector<Instance *> _instances;
	std::map<Segment, int> _segment2Idx;
	std::vector<Interface> _faces;
	std::vector<AtomPair> _connectionPairs;
	std::vector<AtomPair> _fiducialConnectionPairs;
	std::map<int, MatId> _seqToId;
	std::map<Atom *, int> _atom2Seq;
	std::vector<MatId> _matIds;

	Eigen::MatrixXf _positions;
	Eigen::MatrixXf _barycentrics;
	Eigen::MatrixXf _targets;
	
	Segment _invariant{-1};
	int _snapColumnFrom{0};
	bool _ready = false;
};

#endif
