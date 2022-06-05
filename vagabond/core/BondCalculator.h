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

#ifndef __vagabond__BondCalculator__
#define __vagabond__BondCalculator__

#include <climits>
#include <vector>
#include <queue>
#include "Handler.h"
#include "TorsionBasis.h"

class Atom;
class BondSequenceHandler;
class MapTransferHandler;
class PointStoreHandler;
class MapSumHandler;
class ForceField;
class Sampler;

/** \class BondCalculator
 *  Master class for the internal Vagabond engine.
 *
 * \image html vagabond/doc/bondcalculator_class_connections.png width=1200px
 */

class BondCalculator : public Handler
{
public:
	BondCalculator();
	~BondCalculator();
	
	/** Extent of BondCalculator pipeline before exiting and producing a 
	 *  Result. */
	enum PipelineType
	{
		PipelineNotSpecified, /**< unknown pipeline */

		/** return array of positions after calculating atom positions */
		PipelineAtomPositions, 

		/** return maps after calculating 3D map */
		PipelineCalculatedMaps, 

		/** return after calculating correlation of calculated 3D map with 
		 * reference 3D map */
		PipelineCorrelation,
	};
	
	int submitJob(Job &job);
	void submitResult(Result *result);
	
	/** Set limits for which atoms should be used for output results such
	 *  as deviation calculations for positions. All atom positions will
	 *  nevertheless be calculated.
	 *  @param min minimum value for depth to be considered
	 *  @param max maximum value for depth to be considered */
	void setMinMaxDepth(int min, int max)
	{
		if (_minDepth != min || _maxDepth != max)
		{
			_changedDepth = true;
		}

		_minDepth = min;
		_maxDepth = max;
	}
	
	/** Set limits for what branches should be excluded for output results.
	 * e.g. set to 1 to include single-atom branches but not longer sidechains 
	 * @param max maximum atoms down side-branch to be considered */
	void setMaxSideDepth(int max)
	{
		if (_sideMax != max)
		{
			_changedDepth = true;
		}

		_sideMax = max;
	}

	/** Returns vector of booleans corresponding to each scalar in the
	 *  custom vector which is provided to the calculator. True if this
	 *  vector directly affects an atom in provided depth range. */
	std::vector<bool> depthLimitMask();
	
	void setTotalSamples(size_t total)
	{
		_totalSamples = total;
	}
	
	void setPipelineType(PipelineType type)
	{
		_type = type;
	}
	
	/** ignored after setup() is called.
	 * 	@param max must be explicitly specified as non-zero positive */
	void setMaxSimultaneousThreads(size_t max)
	{
		_maxThreads = max;
	}
	
	/** @param size UINT_MAX if unlimited, otherwise maximum bytes as limit 
	 * for map calculations */
	void setMaxMemoryGuideline(unsigned long max)
	{
		_maxMemory = max;
	}
	
	void setTorsionBasisType(TorsionBasis::Type type)
	{
		_basisType = type;
	}
	
	void addAnchorExtension(Atom *atom, size_t bondCount = UINT_MAX);

	void setup();
	void start();
	void finish();

	const size_t maxCustomVectorSize() const;

	/** set whether BondSequences should include hydrogen atoms.
	 *  Ignored after setup() is called. */
	void setIgnoreHydrogens(bool ignore)
	{
		_ignoreHydrogens = ignore;
	}
	
	BondSequenceHandler *sequenceHandler()
	{
		return _sequenceHandler;
	}
	
	MapTransferHandler *mapHandler()
	{
		return _mapHandler;
	}
	
	MapSumHandler *sumHandler()
	{
		return _sumHandler;
	}
	
	void setSampler(Sampler *sampler);
	
	void setForceField(ForceField *ff)
	{
		_forceField = ff;
	}
	
	ForceField *const forceField() const
	{
		return _forceField;
	}
	
	struct AnchorExtension
	{
		Atom *atom;
		size_t count;
	};

	Job *acquireJob();
	Result *acquireResult();
private:
	void sanityCheckPipeline();
	void sanityCheckThreads();
	void sanityCheckJob(Job &job);
	void sanityCheckDepthLimits();
	void setupSequenceHandler();
	void setupPointHandler();
	void setupMapTransferHandler();
	void setupMapSumHandler();
	void prepareThreads();

	PipelineType _type;
	size_t _maxThreads = 0;
	size_t _maxMemory = 0;
	size_t _totalSamples = 0;
	
	int _minDepth = 0;
	int _maxDepth = INT_MAX;
	int _sideMax = INT_MAX;
	bool _changedDepth = false;
	bool _ignoreHydrogens = false;
	
	std::atomic<long int> _max_id;
	std::atomic<long int> _running;
	
	Pool<Job *> _jobPool;
	Pool<Result *> _resultPool;
	Pool<Result *> _recyclePool;

	BondSequenceHandler *_sequenceHandler = nullptr;
	PointStoreHandler *_pointHandler = nullptr;
	MapTransferHandler *_mapHandler = nullptr;
	MapSumHandler *_sumHandler = nullptr;
	TorsionBasis::Type _basisType = TorsionBasis::TypeSimple;
	Sampler *_sampler = nullptr;
	ForceField *_forceField = nullptr;
	
	std::vector<AnchorExtension> _atoms;
};

#endif
