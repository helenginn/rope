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

class Atom;

class BondSequenceHandler;

struct MiniJob;

class BondSequence
{
public:
	BondSequence(BondSequenceHandler *handler = nullptr);
	~BondSequence();
	
	size_t addedAtomsCount()
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
	
	void cleanUpToIdle();
	void setMiniJobInfo(MiniJob *mini);
	void printState();
	
	MiniJob *miniJob();

	/* extend the atom graph for bond sequence generation
	 * 	@param atom beginning anchor atom
	 * 	@param count maximum bond depth to consider */
	void addToGraph(Atom *atom, size_t count = UINT_MAX);
	
	void multiplyUpBySampleCount();
	
	void prepareForIdle();

	void calculate();
	std::vector<Atom::WithPos> &extractPositions();

	void setSampleCount(int count)
	{
		_sampleCount = count;
	}
	
	size_t sampleCount()
	{
		return _sampleCount;
	}
	
	SequenceState state();
private:
	struct AtomGraph
	{
		Atom *atom;
		Atom *parent;
		int depth;
		int maxDepth;
		std::vector<AtomGraph *> children;
		
		bool operator<(AtomGraph &other) const
		{
			return depth < other.depth;
		}
	};
	
	struct AtomBlock
	{
		Atom *atom;
		char element[3];
		int nBonds;
		bool flag;
		glm::mat4x4 coordination;
		glm::vec3 inherit;
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
		
		const glm::vec3 child_position(int i) const
		{
			return glm::vec3(wip[i]);
		}
	};
	
	std::vector<AtomBlock> _blocks;

	void generateBlocks();
	void generateAtomGraph(Atom *atom, size_t count);
	void addGraph(AtomGraph *graph);
	void calculateMissingMaxDepths();
	void sortGraphChildren();
	void assignAtomToBlock(int idx, Atom *atom);
	void assignAtomsToBlocks();
	void fillMissingWriteLocations();
	void fixBlockAsGhost(int idx);
	void removeGraphs();

	void resetFlag(int idx);
	void calculateBlock(int idx);
	void fetchTorsion(int idx);
	void preparePositions();


	std::vector<Atom *> _atoms;
	std::vector<Atom *> _anchors;
	std::vector<AtomGraph *> _graphs;
	std::map<Atom *, AtomGraph *> _atom2Graph;
	std::map<Atom *, int> _atom2Block;

	std::mutex _mutex;
	size_t _sampleCount;
	size_t _addedAtomsCount;
	SequenceState _state;
	
	void setMiniJob(MiniJob *job);
	void signal(SequenceState newState);

	MiniJob *_miniJob;
	BondSequenceHandler *_handler;
	std::vector<Atom::WithPos> _posAtoms;
};

#endif
