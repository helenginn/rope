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

#include <vector>
#include <map>

class SimplexEngine
{
public:
	SimplexEngine();
	virtual ~SimplexEngine() {};

	void setDimensionCount(int dims)
	{
		_dims = dims;
	}
	
	void chooseStepSizes(float *steps)
	{
		_steps = steps;
	}
	
	void setMaxJobsPerVertex(int maxJobs)
	{
		_maxJobs = maxJobs;
	}

	void run();


	void finish()
	{
		_finish = true;
	}
protected:
	typedef std::vector<float> Point;

	const Point &bestPoint() const;
	virtual int sendJob(Point &trial);
	virtual int awaitResult(double *eval);
	
	void printPoint(Point &point);
	std::atomic<bool> _finish;
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
		Point vertex;
		double eval;
		std::map<int, Point> tickets;
		Decision decision;
		
		const bool operator<(TestPoint &other) const
		{
			return (eval < other.eval);
		}
	};

	void allocateResources();
	void resetVertex(TestPoint &point);
	void reorderVertices();
	Point scaleThrough(Point &p, Point &q, float k);

	bool awaitResults();

	void sendStartingJobs();
	void sendDecidedJobs();
	void sendReflectionJob(int i);
	void sendContractionJob(int i);
	void sendExpansionJob(int i);
	void sendShrinkJobs();
	void cycle();
	void findCentroid();

	std::vector<TestPoint> _points;
	TestPoint _centroid;

	int _dims = 0;
	int _maxJobs = 0;
	float *_steps = nullptr;
};

#endif
