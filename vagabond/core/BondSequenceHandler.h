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

class Sampler;
class BondSequence;
class BondCalculator;
class MapTransferHandler;
class PointStoreHandler;
class ThreadCalculatesBondSequence;
class ThreadExtractsBondPositions;
	
class BondSequenceHandler : public Handler, public HasBondSequenceCustomisation
{
public:
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

	BondSequence *sequence(int i) 
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

	void signalToHandler(BondSequence *seq, SequenceState state);

	BondSequence *acquireSequence(SequenceState state);

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
