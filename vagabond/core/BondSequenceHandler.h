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
#include "BondSequence.h"
#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include "HasBondSequenceCustomisation.h"
#include "engine/ElementTypes.h"
#include "Result.h"

class Sampler;
class BondSequence;
class BondCalculator;
class MapTransferHandler;
class PointStoreHandler;
class ThreadCalculatesBondSequence;
class ThreadExtractsBondPositions;
template <typename I, typename O> class Task;
	
namespace Flag
{
	enum Calc
	{
		NoCalc = 1 << 0,
		DoTorsions = 1 << 1,
		DoPositions = 1 << 2,
		DoSuperpose = 1 << 3,
	};

	enum Extract
	{
		NoExtract = 1 << 0,
		AtomVector = 1 << 1,
		AtomMap = 1 << 2,
		Deviation = 1 << 3,
	};
}


class BondSequenceHandler : public Handler, public HasBondSequenceCustomisation
{
public:
	BondSequenceHandler(int totalSeq);

	BondSequenceHandler(BondCalculator *calculator = nullptr);
	~BondSequenceHandler();
	
	void setPointStoreHandler(PointStoreHandler *handler)
	{
		_pointHandler = handler;
	}
	
	void setMapTransferHandler(MapTransferHandler *handler)
	{
		_mapHandler = handler;
	}

	BondCalculator *calculator()
	{
		return _calculator;
	}
	
	TorsionBasis *torsionBasis() const;

	void addAnchorExtension(AnchorExtension ext);
	
	/** set up resources which are needed for calculations */
	void setup();
	
	/** set up workers and corresponding threads, begin calculations */
	void start();
	
	/** stop all work, join up threads and delete threads/workers */
	void finish();

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
	BaseTask *extract(Flag::Extract flags,
	                  Task<Result, void *> *submit_result,
	                  Task<Ticket, Ticket> *hook,
	                  Task<Ticket, Deviation> **dev = nullptr,
	                  Task<Ticket, AtomPosList *> **list = nullptr,
	                  Task<Ticket, AtomPosMap *> **map = nullptr);

	// final_hook is the final job before extracting
	void calculate(int ticket, Flag::Calc flags,
	               const std::vector<float> &parameters,
	               BaseTask **first_task,
	               Task<Ticket, Ticket> **final_hook);

	// let go of sequence before continuing
	Task<Ticket, void *> *letGo();

	void signalToHandler(BondSequence *seq, SequenceState state);

	BondSequence *acquireSequence(SequenceState state);
	BondSequence *acquireSequenceOrNull();

	/** Changes which atoms are included for calculation of position
	 * deviation 
	 * 	@param min minimum depth from anchor position
	 * 	@param max maximum depth from anchor position  */
	void imposeDepthLimits(int min, int max, bool limit_max);

	int activeTorsions();
	void prepareSequences();
private:
	void sanityCheckThreads();
	void prepareSequenceBlocks();
	void prepareThreads();
	void calculateThreads(int max);

	size_t _totalSequences = 0;

	std::map<SequenceState, Pool<BondSequence *> > _pools;

	std::vector<AnchorExtension> _atoms;

	/* Sequences to manage calculations */
	std::vector<BondSequence *> _sequences;
	BondCalculator *_calculator = nullptr;
	std::map<std::string, int> _elements;
	MapTransferHandler *_mapHandler = nullptr;
	PointStoreHandler *_pointHandler = nullptr;
	CoordManager *_manager = nullptr;
};

#endif
