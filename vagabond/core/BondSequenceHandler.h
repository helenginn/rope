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

#ifndef __vagabond__BondSequenceHandler__
#define __vagabond__BondSequenceHandler__

#include <thread>
#include "engine/Handler.h"
#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include "HasBondSequenceCustomisation.h"
#include "engine/ElementTypes.h"
#include "Result.h"
#include "ParamSet.h"

class Sampler;
class Grapher;
class BondSequence;
class BondCalculator;
class CompareDistances;
class MapTransferHandler;
class ThreadCalculatesBondSequence;
class ThreadExtractsBondPositions;
template <typename I, typename O> class Task;
	

class BondSequenceHandler : public Handler, public HasBondSequenceCustomisation
{
public:
	BondSequenceHandler(int totalSeq);

	~BondSequenceHandler();
	
	const Grapher &grapher() const;
	
	TorsionBasis *torsionBasis() const;

	void addAnchorExtension(AnchorExtension ext);
	
	/** set up resources which are needed for calculations */
	void setup();

	BondSequence *sequence(int i = 0) 
	{
		return _sequences[i];
	}
	
	int sequenceCount()
	{
		return _sequences.size();
	}
	
	int threadCount()
	{
		return _threads;
	}
	
	CoordManager *manager()
	{
		return _manager;
	}

	// extracting meaningful information from sequence, will return
	// a pointer to the let_go if further tasks are required
	Task<BondSequence *, void *> *extract
	(Flag::Extract flags,
	 Task<Result, void *> *submit_result,
	 CalcTask *hook,
	 Task<BondSequence *, Deviation> **dev = nullptr,
	 Task<BondSequence *, AtomPosList *> **list = nullptr,
	 Task<BondSequence *, AtomPosMap *> **map = nullptr);

	// final_hook is the final job before extracting
	void calculate(Flag::Calc flags, const std::vector<float> &parameters,
	               BaseTask **first_task, CalcTask **final_hook,
	               const CoordManager *specific_manager = nullptr);

	// pretty optional, preparing lists of element positions for maps
	void positionsForMap(Task<BondSequence *, BondSequence *> *hook,
	                     Task<BondSequence *, void *> *letgo,
	                     std::map<std::string, GetEle> &eleTasks);

	Task<BondSequence *, void *> *
	extract_compare_distances(Task<Result, void *> *submit_result,
	                          CompareDistances *compare,
	                          CalcTask *hook, bool show = true);

	// let go of sequence before continuing
	Task<BondSequence *, void *> *letGo();

	void signalToHandler(BondSequence *seq, SequenceState state);

	BondSequence *acquireSequenceOrNull();

	/** Changes which atoms are included for calculation of position
	 * deviation 
	 * 	@param min minimum depth from anchor position
	 * 	@param max maximum depth from anchor position
	 *  @param limit_max treat max like a hard stop when true, otherwise
	 *  maximum number of connections to min  */
	void imposeDepthLimits(int min, int max, bool limit_max);

	void clearDepthLimits()
	{
		imposeDepthLimits(0, INT_MAX, false);
	}
	
	ParamSet flaggedParameters();
	
	const std::map<std::string, int> &elementList() const
	{
		return _elements;
	}

	int activeTorsions();
	void prepareSequences();
private:
	void prepareSequenceBlocks();

	size_t _totalSequences = 0;

	std::map<SequenceState, Pool<BondSequence *> > _pools;

	std::vector<AnchorExtension> _atoms;

	/* Sequences to manage calculations */
	std::vector<BondSequence *> _sequences;
	CoordManager *_manager = nullptr;
	std::map<std::string, int> _elements;
};

#endif
