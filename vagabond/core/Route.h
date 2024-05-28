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

#ifndef __vagabond__Route__
#define __vagabond__Route__

#include <atomic>
#include "StructureModification.h"
#include "engine/CoordManager.h"
#include "NonCovalents.h"
#include "Responder.h"
#include "RTMotion.h"
#include "RTPeptideTwist.h"
#include "paths/Scores.h"
#include "Bin.h"

class Grapher;
class Separation;
struct AtomGraph;
struct GradientPath;
class ResidueTorsion;
class TorsionCluster;
class RouteValidator;
class EnergyTorsions;
class PairwiseDeviations;

class Route : public StructureModification, public HasResponder<Responder<Route> >
{
public:
	Route(Instance *from, Instance *to, const RTAngles &list);
	Route(const RTAngles &list);
	
	void addLinkedInstances(Instance *from, Instance *to);
	virtual ~Route();

	virtual void setup();

	enum CalcOptions
	{
		None = 0,
		Pairwise = 1 << 0,
		NoHydrogens = 1 << 1,
		TorsionEnergies = 1 << 2,
		VdWClashes = 1 << 3,
		PerResidue = 1 << 4,
	};

	friend std::ostream &operator<<(std::ostream &ss, const CalcOptions &opts);

	/*
	void submitJob(float frac, bool show = true, 
	               const CalcOptions &options = None,
	               int ticket = 1);
	*/

	void submitToShow(float frac);
	
	void setNonCovalents(NonCovalents *noncovs)
	{
		_noncovs = noncovs;
	}

	float submitJobAndRetrieve(float frac, bool show = true);
	
	void shouldUpdateAtoms(bool update)
	{
		_updateAtoms = update;
	}
	
	void setCycles(int cycles)
	{
		_cycles = cycles;
	}
	
	void setDestinationInstance(Instance *inst)
	{
		_endInstance = inst;
	}
	
	Instance *endInstance()
	{
		return _endInstance;
	}
	
	float clashScore() 
	{
		return _clash;
	}
	
	float momentumScore() 
	{
		return _momentum;
	}
	
	void setActivationEnergy(float energy) 
	{
		_activationEnergy = energy;
	}
	
	float activationEnergy() 
	{
		return _activationEnergy;
	}

	void setTorsionEnergy(float energy) 
	{
		_torsionEnergy = energy;
	}
	
	float torsionEnergy() 
	{
		return _torsionEnergy;
	}

	/* to be called on separate thread */
	static void calculate(Route *me);
	
	bool calculating()
	{
		return _calculating;
	}
	
	void prepareCalculate()
	{
		_calculating = true;
	}
	
	virtual void prepareForAnalysis() {};
	
	virtual void finishRoute()
	{
		_finish = true;
	}
	
	const size_t wayPointCount() const
	{
		return _motions.size();
	}
	
	WayPoints &wayPoints(int idx)
	{
		return _motions.storage(idx).wp;
	}
	
	void setWayPoints(int idx, const WayPoints &wps) 
	{
		_motions.storage(idx).wp = wps;
	}
	
	PeptideTwist &twist(int i)
	{
		return _twists.storage(i);
	}
	
	size_t twistCount() const
	{
		return _twists.size();
	}
	
	Motion &motion(int i)
	{
		return _motions.storage(i);
	}
	
	size_t motionCount() const
	{
		return _motions.size();
	}
	
	void clearCustomisation();
	
	Parameter *parameter(int i)
	{
		if (i < 0) return nullptr;
		return _motions.rt(i).parameter();
	}
	
	std::vector<ResidueTorsion> residueTorsions();

	ResidueTorsion &residueTorsion(int i)
	{
		return _motions.rt(i);
	}
	
	void setFlips(std::vector<int> &idxs, std::vector<int> &fs);
	
	int indexOfParameter(Parameter *t);

	void bestGuessTorsions();
	
	const RTMotion &motions() const
	{
		return _motions;
	}
	
	void setMotions(const RTMotion &motions)
	{
		_motions = motions;
	}

	void prepareTwists();
	
	const RTPeptideTwist &twists() const
	{
		return _twists;
	}
	
	void setTwists(const RTPeptideTwist &twists);
	
	size_t instanceCount()
	{
		return _pairs.size();
	}

	Instance *startInstance(int i)
	{
		return _pairs[i].start;
	}

	Instance *endInstance(int i)
	{
		return _pairs[i].end;
	}
	
	void setChosenFrac(const float &frac)
	{
		_chosenFrac = frac;
	}
	
	bool doingQuadratic()
	{
		return _jobLevel == 0;
	}

	bool doingCubic()
	{
		return _jobLevel >= 1;
	}
	
	bool doingSides()
	{
		return _jobLevel >= 2;
	}
	
	bool doingClashes()
	{
		return _jobLevel >= 2;
	}
	
	bool doingHydrogens()
	{
		return _jobLevel >= 3;
	}
	
	bool lastJob()
	{
		return _jobLevel >= 4;
	}

	const int &jobLevel() const
	{
		return _jobLevel;
	}
	
	void setJobLevel(int level)
	{
		_jobLevel = level;
	}
	
	void setMaximumFlipTrial(const float &trials)
	{
		_maxFlipTrial = trials;
	}

	const float &maximumClashDistance() const
	{
		return _maxClashDistance;
	}
	
	void setMaximumClashDistance(const float &distance)
	{
		_maxClashDistance = distance;
	}

	const float &maximumMomentumDistance() const
	{
		return _maxMomentumDistance;
	}
	
	void setMaximumMomentumDistance(const float &distance)
	{
		_maxMomentumDistance = distance;
	}

	void setHash(const std::string &hash = "");
	const std::string &hash() const
	{
		return _hash;
	}
	
	AtomGroup *all_atoms();
protected:
	typedef std::function<bool(int idx)> ValidateParam;


	virtual void prepareResources();
	const Grapher &grapher() const;
	GradientPath *submitGradients(const CalcOptions &options, int order,
	                              const ValidateParam &validate = {},
	                              BondSequenceHandler *handler = nullptr);

	void submitValue(const CalcOptions &options, int steps,
	                 BondSequenceHandler *handler);

	virtual void doCalculations() {};
	
	float &destination(int i)
	{
		return _motions.storage(i).angle;
	}
	
	void setFlip(int idx, bool flip)
	{
		_motions.storage(idx).flip = flip;
	}
	
	bool flip(int i) const
	{
		return _motions.storage(i).flip;
	}

	void bestGuessTorsion(int i);
	
	std::atomic<bool> _finish{false};

	void prepareDestination();
	typedef std::function<Motion(Parameter *const &param, 
	                             ResidueTorsion &rt)> MakeMotion;
	void getParametersFromBasis(const MakeMotion &make_mot);
	void prepareParameters();

	AtomGraph *grapherForTorsionIndex(int idx);

	float getTorsionAngle(int i);
	
	int cycles()
	{
		return _cycles;
	}

	bool _updateAtoms = true;
	int _cycles = -1;

	void updateAtomFetch(BondSequenceHandler *const &handler);
	void prepareEnergyTerms();
	RTMotion _motions;
	RTPeptideTwist _twists;
	int _jobLevel = 0;
	std::vector<int> _activeParams;
	
	std::set<ResidueId> _ids;
	
	void setScores(float &momentum, float &clash)
	{
		_momentum = momentum;
		_clash = clash;
		_gotScores = true;
	}

	float _activationEnergy = FLT_MAX;
	float _momentum = FLT_MAX;
	float _clash = FLT_MAX;
	float _vdwEnergy = FLT_MAX;
	float _torsionEnergy = FLT_MAX;
	bool _gotScores = false;

	BondSequenceHandler *_mainChainSequences = nullptr;
	BondSequenceHandler *_hydrogenFreeSequences = nullptr;

	void unlockAll();
	int _maxFlipTrial = 0;
	int _order = 2;

	Bin<ByResidueResult> _perResBin;
	Bin<GradientPath> _gradientBin;
	
	float _chosenFrac = 0.5;
private:
	bool _calculating = false;
	
	NonCovalents *_noncovs = nullptr;
	
	void addToAtomPosMap(AtomPosMap &map, Result *r);
	void calculateAtomDeviations(Score &score);
	
	float _maxMomentumDistance = 8.f;
	float _maxClashDistance = 15.f;
	
	struct Helpers
	{
		BondSequence *seq = nullptr;
		PairwiseDeviations *pw = nullptr;
		EnergyTorsions *et = nullptr;
		Separation *sep = nullptr;
	};

	friend void setup_helpers(Route::Helpers &helpers, 
	                          BondSequence *seq, float distance);
	
	void deleteHelpers();
	
	struct InstancePair
	{
		Instance *start = nullptr;
		Instance *end = nullptr;
	};
	
	std::vector<InstancePair> _pairs;
	std::map<BondSequenceHandler *, Helpers> _helpers;
	std::map<Parameter *, int> _parameter2Idx;
	
	Instance *_endInstance = nullptr;
	RTAngles _source;
	
	std::string _hash;
	bool _setup = false;

};

inline std::ostream &operator<<(std::ostream &ss, const Route::CalcOptions &opts)
{
	if (opts & Route::Pairwise) ss << "Pairwise ";
	if (opts & Route::NoHydrogens) ss << "NoHydrogens ";
	if (opts & Route::TorsionEnergies) ss << "TorsionEnergies ";
	if (opts & Route::VdWClashes) ss << "VdWClashes ";
	if (opts & Route::PerResidue) ss << "PerResidue ";
	return ss;
}

#endif
