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

#define _USE_MATH_DEFINES
#include <math.h>
#include <climits>
#include <vector>
#include <queue>
#include "engine/Handler.h"
#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include "HasBondSequenceCustomisation.h"
#include "FFProperties.h"

class BondSequenceHandler;
class SurfaceAreaHandler;
class ForceFieldHandler;
class MapTransferHandler;
class CorrelationHandler;
class PointStoreHandler;
class SolventHandler;
class MapSumHandler;

class PositionSampler;
class Diffraction;
class ForceField;
class Grapher;
class Sampler;
class Atom;

/** \class BondCalculator
 *  Master class for the internal Vagabond engine.
 *
 * \image html vagabond/doc/bondcalculator_class_connections.png width=1200px
 */

class BondCalculator : public Handler, public HasBondSequenceCustomisation
{
public:
	BondCalculator();
	~BondCalculator();
	
	/** Extent of BondCalculator pipeline before exiting and producing a 
	 *  Result. */
	enum PipelineType
	{
		PipelineNotSpecified = 0, /**< unknown pipeline */

		/** return array of positions after calculating atom positions */
		PipelineAtomPositions = 1 << 0, 

		/** return maps after calculating 3D map */
		PipelineCalculatedMaps = 1 << 1, 
		
		/** returns forcefield-related calculations from atom positions */
		PipelineForceField = 1 << 2,

		/** return after calculating correlation of calculated 3D map with 
		 * reference 3D map */
		PipelineCorrelation = 1 << 3,

		/** returns solvent-accessible surface area calculation in Angstroms^2 */
		PipelineSolventSurfaceArea = 1 << 4,

		/** calculates a solvent mask for maps */
		PipelineSolventMask = 1 << 5,
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
	
	/** Returns vector of booleans corresponding to each scalar in the
	 *  custom vector which is provided to the calculator. True if this
	 *  vector directly affects an atom in provided depth range. */
	std::vector<bool> depthLimitMask();
	
	void setPipelineType(PipelineType type)
	{
		_type = type;
	}
	
	const PipelineType &pipelineType() const
	{
		return _type;
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
	void addAnchorExtension(AnchorExtension ext);
	
	Atom *anchorAtom(int i)
	{
		return _atoms[i].atom;
	}
	
	const size_t anchorAtomCount() const
	{
		return _atoms.size();
	}

	void setup();
	void start();
	void finish();
	
	const size_t maxCustomVectorSize() const;
	
	BondSequenceHandler *sequenceHandler()
	{
		return _sequenceHandler;
	}
	
	BondSequence *sequence(int i = 0);
	TorsionBasis *torsionBasis();
	const Grapher &grapher() const;
	
	void setReferenceDensity(OriginGrid<fftwf_complex> *dens)
	{
		_refDensity = dens;
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
	
	SurfaceAreaHandler *surfaceHandler()
	{
		return _surfaceHandler;
	}
	
	ForceFieldHandler *const forceFieldHandler() const
	{
		return _ffHandler;
	}
	
	void setForceFieldProperties(FFProperties &props)
	{
		_props = props;
	}

	Job *acquireJob();
	Result *acquireResult();
	void reset();
private:
	void sanityCheckPipeline();
	void sanityCheckThreads();
	void sanityCheckJob(Job &job);
	void sanityCheckDepthLimits();
	void setupSequenceHandler();
	void setupPointHandler();
	void setupSolventHandler();
	void setupMapTransferHandler();
	void setupMapSumHandler();
	void setupForceFieldHandler();
	void setupSurfaceAreaHandler();
	void prepareThreads();

	PipelineType _type;
	size_t _maxMemory = 0;
	
	int _minDepth = 0;
	int _maxDepth = INT_MAX;
	bool _changedDepth = false;
	
	Pool<Job *> _jobPool;
	ExpectantPool<Result *> _resultPool;
	Pool<Result *> _recyclePool;

	BondSequenceHandler *_sequenceHandler = nullptr;
	MapTransferHandler *_mapHandler = nullptr;
	CorrelationHandler *_correlHandler = nullptr;
	SurfaceAreaHandler *_surfaceHandler = nullptr;
	PointStoreHandler *_pointHandler = nullptr;
	ForceFieldHandler *_ffHandler = nullptr;
	SolventHandler *_solventHandler = nullptr;
	MapSumHandler *_sumHandler = nullptr;

	TorsionBasis::Type _basisType = TorsionBasis::TypeSimple;
	Sampler *_sampler = nullptr;
	FFProperties _props{};
	
	OriginGrid<fftwf_complex> *_refDensity = nullptr;

	void setupCorrelationHandler();
	
	bool _started = false;
	std::vector<AnchorExtension> _atoms;
};

#endif
