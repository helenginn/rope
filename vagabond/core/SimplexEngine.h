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

#ifndef __vagabond__SimplexEngine__
#define __vagabond__SimplexEngine__

#include <atomic>
#include <ostream>
#include <vector>
#include <map>

#include "Engine.h"

template <class X, class Y> class Task;
class MultiEngine;
class BaseTask;

class SimplexEngine : public Engine
{
public:
	SimplexEngine(RunsEngine *ref);
	virtual ~SimplexEngine() {};
	
	void chooseStepSizes(std::vector<float> &steps)
	{
		_steps = steps;
	}
	
	void setMaxJobsPerVertex(int maxJobs)
	{
		_maxJobs = maxJobs;
	}

	Task<void *, void *> *taskedRun(MultiEngine *ms);

	virtual void finish();
protected:
	virtual void run();
	void preRun();

	typedef std::vector<float> SPoint;

	const SPoint &bestPoint() const;

	void printPoint(SPoint &point);
	void printSimplex();

	std::atomic<bool> _finish{false};

private:
	
	enum Decision
	{
		Nothing,
		ShouldContract,
		ShouldReflect,
		ShouldExpand,
	};

	struct TestPoint
	{
		SPoint vertex;
		double eval;
		std::map<int, SPoint> tickets;
		Decision decision;
		
		const bool operator<(TestPoint &other) const
		{
			return (eval < other.eval);
		}
		
		friend std::ostream &operator<<(std::ostream &ss, 
		                                const TestPoint &tp)
		{
			ss << "Point: (";
			const int max_ = 3;
			int max = (tp.vertex.size() > max_ ? max_ : tp.vertex.size());
			for (size_t i = 0; i < max; i++)
			{
				ss << tp.vertex[i] << (i < max - 1 ? ", " : "");
			}
			if (max > max_) ss << "...";
			ss << "), eval: " << tp.eval;

			return ss;
		}
	};

	bool reachedConvergence();
	void allocateResources();
	void resetVertex(TestPoint &point);
	void reorderVertices();
	SPoint scaleThrough(SPoint &p, SPoint &q, float k);

	void collateResults();

	void requestReflection();
	void sendStartingJobs();
	void submitShrink();
	void cycle();
	void handleJobs();
	void findCentroid();

	void replaceReflection();
	void submitExpansion();
	void handleExpansion();
	void submitContraction(float eval, float worst_score, 
	                       float second_worst_score);
	void handleContraction();

	Task<void *, void *> *taskedHandleJobs(Task<void *, void *> *before,
	                                       MultiEngine *ms);
	void handleShrink(MultiEngine *ms, Task<void *, void *> *&first);

	void taskedCycle(Task<void *, void *> *before, MultiEngine *ms);
	void prepareCycle(MultiEngine *ms);
	
	int pointCount()
	{
		return _points.size();
	}

	std::vector<TestPoint> _points;
	TestPoint _centroid{};

	TestPoint &worst();
	TestPoint &second_worst();
	SPoint _reflected;
	SPoint _expanded;
	SPoint _contracted;
	float _compare = 0;
	float _lastEval = 0;

	int _maxJobs = 1;
	int _count = 0;
	int _shrinkCount = 0;
	std::vector<float> _steps;
	
	struct ScoreTrio
	{
		float worst;
		float second_worst;
		float best;
	};

	ScoreTrio _trio{};

	std::function<void *(void *)> _cycle{};
	std::function<void *(void *)> _handle_expand{};
	std::function<void *(void *)> _declare_done{};
	std::function<void *(void *)> _handle_contract{};
	std::function<void *(void *)> _decide_what_to_do{};
};

#endif
