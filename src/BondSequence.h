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
#include "glm_import.h"
#include "Atom.h"
#include "BondSequenceHandler.h"
#include "BondTorsion.h"

class Atom;

class BondSequenceHandler;

struct MiniJob;

class BondSequence
{
public:
	BondSequence(BondSequenceHandler *handler = nullptr);
	~BondSequence();
	
	const size_t addedAtomsCount() const
	{
		return _addedAtomsCount;
	}
	
	Atom *atomForBlock(int i)
	{
		return _blocks[i].atom;
	}
	
	int numBondsForBlock(int i)
	{
		return _blocks[i].nBonds;
	}
	
	size_t blockCount()
	{
		return _blocks.size();
	}
	
	int firstBlockForAtom(Atom *atom);
	
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
	
	bool checkAtomGraph(int i) const;
	
	size_t atomGraphCount() const
	{
		return _graphs.size();
	}
	
	void cleanUpToIdle();
	void setMiniJobInfo(MiniJob *mini);
	void printState();
	void removeGraphs();
	void removeTorsionBasis();
	
	const size_t maxDepth() const
	{
		return _maxDepth;
	}
	
	void setTorsionBasisType(TorsionBasis::Type type)
	{
		_basisType = type;
	}
	
	TorsionBasis *torsionBasis() const
	{
		return _torsionBasis;
	}
	
	MiniJob *miniJob();

	/* extend the atom graph for bond sequence generation
	 * 	@param atom beginning anchor atom
	 * 	@param count maximum bond depth to consider */
	void addToGraph(Atom *atom, size_t count = UINT_MAX);
	
	void multiplyUpBySampleCount();
	void reflagDepth(int min, int max);
	std::vector<bool> atomMask();
	
	void prepareForIdle();
	void reset()
	{
		_state = SequenceInPreparation;
	}

	void calculate();
	std::vector<Atom::WithPos> &extractPositions();
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
	
	std::string atomGraphDesc(int i);
private:
	struct AtomGraph
	{
		Atom *atom;
		Atom *parent;
		Atom *grandparent;
		int depth;
		int maxDepth;
		BondTorsion *torsion;
		int torsion_idx;
		std::vector<AtomGraph *> children;
		
		bool operator<(const AtomGraph &other) const
		{
			/* otherwise go for tinier branch points first */
			return maxDepth < other.maxDepth;
		}
	};

	static bool atomgraph_less_than(const AtomGraph *a, const AtomGraph *b);
	
	struct AtomBlock
	{
		Atom *atom;
		char element[3];
		int nBonds;
		bool flag = true;
		glm::vec3 target;
		glm::mat4x4 coordination;
		glm::vec3 inherit;
		int torsion_idx;
		float torsion;
		glm::mat4x4 basis;
		glm::mat4x4 wip;
		int write_locs[4];
		
		const glm::vec3 parent_position() const
		{
			return inherit;
		}
		
		const glm::vec3 my_position() const
		{
			return glm::vec3(basis[3]);
		}
		
		const glm::vec3 &target_position() const
		{
			return target;
		}
		
		const glm::vec3 child_position(int i) const
		{
			return glm::vec3(wip[i]);
		}
	};
	
	std::vector<AtomBlock> _blocks;

	void generateBlocks();
	void acquireCustomVector(int sampleNum);
	void generateAtomGraph(Atom *atom, size_t count);
	void addGraph(AtomGraph *graph);
	void calculateMissingMaxDepths();
	void sortGraphChildren();
	void assignAtomToBlock(int idx, Atom *atom);
	void assignAtomsToBlocks();
	void fillMissingWriteLocations();
	void fixBlockAsGhost(int idx, Atom *anchor);
	void makeTorsionBasis();
	void fillInParents();
	void fillTorsionAngles();

	int calculateBlock(int idx);
	void fetchTorsion(int idx);

	std::vector<Atom *> _atoms;
	std::vector<Atom *> _anchors;
	std::vector<AtomGraph *> _graphs;
	std::map<Atom *, AtomGraph *> _atom2Graph;
	std::map<Atom *, int> _atom2Block;

	size_t _sampleCount = 1;
	size_t _addedAtomsCount = 0;
	SequenceState _state = SequenceInPreparation;
	
	void setMiniJob(MiniJob *job);
	void signal(SequenceState newState);
	
	int _startCalc = 0;
	int _endCalc = INT_MAX;
	int _maxDepth = 0;
	int _customIdx = 0;
	CustomVector *_custom = nullptr;
	void checkCustomVectorSizeFits();

	MiniJob *_miniJob = nullptr;
	BondSequenceHandler *_handler = nullptr;
	TorsionBasis *_torsionBasis = nullptr;
	TorsionBasis::Type _basisType;
	std::vector<Atom::WithPos> _posAtoms;
};

#endif
