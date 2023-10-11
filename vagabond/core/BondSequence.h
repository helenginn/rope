// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__BondSequence__
#define __vagabond__BondSequence__

#include <map>
#include <mutex>
#include <vector>
#include <climits>
#include "../utils/glm_import.h"
#include <vagabond/utils/version.h>
#include <vagabond/utils/ConvertIndex.h>
#include "AtomPosMap.h"
#include "ParamSet.h"
#include "programs/RingProgram.h"
#include "HasBondSequenceCustomisation.h"
#include "BondSequenceHandler.h"
#include "engine/JobManager.h"
#include <vagabond/utils/Vec3s.h>
#include "BondTorsion.h"
#include "AtomBlock.h"
#include "Grapher.h"
#include "TorsionBasis.h"
#include "AnchorExtension.h"

class Atom;
class BondSequenceHandler;

class MiniJobSeq;


class BondSequence: public HasBondSequenceCustomisation
{
public:
	BondSequence(BondSequenceHandler *handler = nullptr);
	~BondSequence();
	
	BondCalculator *calculator();
	
	const size_t addedAtomsCount() const
	{
		return _grapher.atoms().size();
	}
	
	const std::vector<Atom *> &addedAtoms() const
	{
		return _grapher.atoms();
	}
	
	std::map<std::string, int> elementList() const;
	
	Atom *atomForBlock(int i) const
	{
		return _blocks[i].atom;
	}
	
	int numBondsForBlock(int i)
	{
		return _blocks[i].nBonds;
	}
	
	const size_t blockCount() const
	{
		return _blocks.size();
	}
	
	glm::vec3 positionForPreviousBlock(int i)
	{
		return _blocks[i].parent_position();
	}
	
	glm::vec3 positionForAtomBlock(int i)
	{
		return _blocks[i].my_position();
	}
	
	glm::vec3 positionForNextBlock(int i, int j)
	{
		return _blocks[i].child_position(j);
	}
	
	const Grapher &grapher() const
	{
		return _grapher;
	}
	
	ParamSet flaggedParameters();

	void cleanUpToIdle();
	void beginJob(Job *job);
	void removeTorsionBasis();
	
	const size_t maxDepth() const
	{
		return _maxDepth;
	}
	
	int activeTorsions() const;
	
	void setTorsionBasisType(TorsionBasis::Type type)
	{
		_basisType = type;
	}
	
	TorsionBasis *torsionBasis() const
	{
		return _torsionBasis;
	}
	
	Job *job()
	{
		return _job;
	}

	/* extend the atom graph for bond sequence generation
	 * 	@param atom beginning anchor atom
	 * 	@param count maximum bond depth to consider */
	void addToGraph(AnchorExtension &atom);
	
	void multiplyUpBySampleCount();
	void reflagDepth(int min, int max, bool limit_max);
	
	void prepareForIdle();

	void reset()
	{
		_state = SequenceInPreparation;
	}

	void calculate(rope::IntToCoordGet coordForIdx);
	void superpose();
	const AtomPosMap &extractPositions();
	void extractVector(AtomPosList &results);
	
	struct ElePos
	{
		glm::vec3 pos;
		char element[3];
	};

	std::vector<ElePos> extractForMap();
	double calculateDeviations();

	void setSampleCount(int count)
	{
		_sampleCount = count;
	}
	
	const size_t sampleCount() const
	{
		return _sampleCount;
	}
	
	SequenceState state();
	void signal(SequenceState newState);
	
	std::vector<AtomBlock> &blocks()
	{
		return _blocks;
	}
	
	const int &nCoords() const
	{
		return _nCoord;
	}

	float fetchTorsion(int idx, const Coord::Get &get);
private:

	struct AtomBlockTodo
	{
		AtomBlock *block;
		int idx;
		int num;
	};

	
	bool atomGraphChildrenOnlyHydrogens(AtomGraph &g);
	void markHydrogenGraphs();

	void generateBlocks();
	void makeTorsionBasis();

	int calculateBlock(int idx, const Coord::Get &get);
	float fetchTorsionForBlock(int idx, const Coord::Get &get);
	void fetchAtomTarget(int idx, const Coord::Get &get);
	
	PositionSampler *posSampler()
	{
		return job()->pos_sampler;
	}
	
	bool positionsAvailable()
	{
		return (posSampler() && _atomPositions.size() && _acceptablePositions);
	}

	Grapher _grapher;
	std::vector<AtomBlock> _blocks;

	size_t _sampleCount = 1;
	int _singleSequence = 0;

	SequenceState _state = SequenceInPreparation;
	
	void setJob(Job *job);
	void printBlock(int idx);
	
	bool _fullRecalc = true;
	int _sampleNum = 0;
	int _startCalc = 0;
	int _endCalc = INT_MAX;
	int _maxDepth = 0;
	
	int _nCoord = 0;

	TorsionBasis *_torsionBasis = nullptr;
	TorsionBasis::Type _basisType;

	Job *_job = nullptr;
	BondSequenceHandler *_handler = nullptr;

	std::vector<RingProgram> _programs;

	bool _usingPrograms = true;

	bool _programsInitialised = false;
	
	Vec3s _atomPositions;
	Floats _bondTorsions;
	bool _acceptablePositions = false;
	
	Index::Convert _convertIndex = Index::identity();
	int _activeTorsions = 0;

	AtomPosMap _posAtoms;
};

#endif
