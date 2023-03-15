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

class SimplexEngine : public Engine
{
public:
	SimplexEngine(RunsEngine *ref);
	virtual ~SimplexEngine() {};

	void setDimensionCount(int dims)
	{
		_dims = dims;
	}
	
	void chooseStepSizes(std::vector<float> &steps)
	{
		_steps = steps;
	}
	
	void setMaxJobsPerVertex(int maxJobs)
	{
		_maxJobs = maxJobs;
	}

	void run();

	virtual void finish();
protected:
	typedef std::vector<float> SPoint;

	const SPoint &bestPoint() const;

	const bool &changedParams() const
	{
		return _changedParams;
	}
	
	void printPoint(SPoint &point);
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

	void allocateResources();
	void resetVertex(TestPoint &point);
	void reorderVertices();
	SPoint scaleThrough(SPoint &p, SPoint &q, float k);

	bool classifyResults();
	void collateResults();

	void sendStartingJobs();
	void sendDecidedJobs();
	void sendReflectionJob(int i);
	void sendContractionJob(int i);
	void sendExpansionJob(int i);
	void sendShrinkJobs();
	void shrink();
	void cycle();
	void pickUpResults();
	void findCentroid();

	std::vector<TestPoint> _points;
	TestPoint _centroid{};

	int _dims = 0;
	int _maxJobs = 1;
	bool _changedParams = false;
	std::vector<float> _steps;
};

#endif
