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

#ifndef __vagabond__PositionShifter__
#define __vagabond__PositionShifter__

#include <map>
#include <list>
#include <atomic>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/OpSet.h>

class PositionShifter
{
public:
	PositionShifter(const glm::mat4x4 &model);
	~PositionShifter();
	
	typedef std::function<glm::vec3()> Getter;
	typedef std::function<void(const glm::vec3 &)> Setter;
	typedef std::function<void()> Tidy;
	typedef std::function<float(void *left, void *right)> TargetFn;

	void addPosition(void *ptr, const Getter &init,
	                 const Getter &getter, const Setter &setter);

	// overrides the default spring target (Euclidean distance between two
	// elements' init positions, gradient()'s get_target) with an
	// externally supplied distance - e.g. a precomputed dissimilarity
	// matrix that has no underlying "real" coordinates to derive init
	// from. Unset by default, so existing callers (init-distance-based
	// layouts, e.g. ProtonNetworkView's 2D arrangement) are unaffected.
	void setTargetFn(const TargetFn &fn)
	{
		_targetFn = fn;
	}

	// per-pair spring weight, multiplied into that pair's contribution in
	// gradient() (never into repulsion - see gradient()'s comment). Unset
	// by default (every pair equally stiff, and fully connected - no
	// pair is ever skipped). The sole mechanism now for "how related are
	// these two elements" - e.g. ProtonNetworkView uses 1.0/0.0 for
	// "within 2 covalent bonds" vs not (replacing this class's former,
	// separate limitSensitivity()/depth=2 relay mechanism), while
	// ClusterPlot uses a continuous 1/target^2 (Kamada-Kawai-style)
	// weight so a fully-connected graph of very unevenly-scaled targets
	// doesn't let the sheer number and magnitude of long-range pairs
	// dominate the gradient and flatten out local structure.
	void setWeightFn(const TargetFn &fn)
	{
		_weightFn = fn;
	}

	// if true, a pair that got a nonzero spring weight above is exempted
	// from repulsion entirely for that pair - repulsion only ever applies
	// between pairs weightFn calls unrelated (weight 0). Restores this
	// class's pre-refactor behaviour (the old depth=2 relay: repulsion
	// only ran over "remaining = all - done", the set NOT reached by the
	// relay), needed by consumers with a strictly binary "related or not"
	// weightFn (ProtonNetworkView, HBondDiagram - both 1.0/0.0 covalent-
	// bond reach): without this, two covalently-bonded atoms - already
	// held at a precise distance by their spring - also got a repulsion
	// kick they never had before, which can drive `actual` towards
	// gradient()'s NaN-guard floor and permanently poison momentum,
	// observed as a dragged atom's position abruptly "snapping" then
	// freezing solid. Left false by default - ClusterPlot's continuous
	// 1/target^2 weight is (in practice) never exactly 0, so it would
	// otherwise lose repulsion for every pair and collapse; it relies on
	// unconditional repulsion, unaffected unless a caller opts in here.
	void setExemptWeightedFromRepulsion(bool exempt)
	{
		_exemptWeightedFromRepulsion = exempt;
	}

	void removePointer(void *ptr);
	
	void includePointer(void *ptr)
	{
		_ptrs.insert(ptr);
	}

	glm::vec3 getPosition(void *ptr);
	void setPosition(void *ptr, glm::vec3 pos);
	
	bool hasPointer(void *ptr)
	{
		return _ptrs.count(ptr);
	}
	
	void setSkip(void *ptr)
	{
		_skip = ptr;
	}
	
	std::mutex &skip_lock()
	{
		return _partialMutex;
	}
	
	void addTidy(const Tidy &tidy)
	{
		_tidyJobs.push_back(tidy);
	}
	
	bool isPaused()
	{
		// _pause is already atomic - no need to contend for _pauseMutex,
		// which the worker thread holds for the whole of move().
		return _pause;
	}
	
	void pause();
	void unpause();
	void run();
	void tidy();
private:
	struct Element
	{
		Getter getter{};
		Setter setter{};

		void *reference;
		Getter raw_getter{};
		Setter raw_setter{};
		glm::vec3 init{};
		glm::vec3 momentum{};
	};
	
	int _num = 0;
	void move();

	// idx indexes into elePtrs/positions, a snapshot of every element's
	// CURRENT getter() (see move()'s own comment on why this is taken
	// once per tick rather than left for gradient() to call getter()
	// itself, which it used to do, redundantly, for the same element up
	// to N times over within a single tick).
	glm::vec3 gradient(size_t idx, const std::vector<Element *> &elePtrs,
	                   const std::vector<glm::vec3> &positions);

	void setupGetterSetters(Element &ele);
	void adjustZ(Element &ele);
	std::map<void *, Element *> _map;
	std::set<void *> _ptrs;

	std::list<Element> _objects;

	void *_skip = nullptr; // not allowed to set/get.

	float _z = 0;
	int _n = 0;

	glm::mat4x4 _model;
	glm::mat4x4 _inv;

	// true if _model (fixed for this instance's whole lifetime - nothing
	// ever updates it after construction) is exactly the identity matrix,
	// letting every getter()/setter() skip an otherwise-pointless 4x4
	// matrix-vector multiply per call. Common for 2D-only consumers (e.g.
	// ClusterPlot) whose Scene never rotates/pans/zooms; essentially
	// never true for ProtonNetworkView's real 3D camera.
	bool _modelIsIdentity = false;

	TargetFn _targetFn{};
	TargetFn _weightFn{};
	bool _exemptWeightedFromRepulsion = false;

	bool _stop{false};
	std::thread *_worker = nullptr;
	std::mutex _partialMutex{};
	std::mutex _pauseMutex{};
	std::condition_variable _waitForPause{};
	std::atomic<bool> _pause{false};
	std::vector<Tidy> _tidyJobs;
};

#endif
