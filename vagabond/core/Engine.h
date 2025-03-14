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

#ifndef __vagabond__Engine__
#define __vagabond__Engine__

#include <mutex>
#include <vector>
#include <map>
#include <cstddef>
#include <cfloat>
#include <functional>

#include "RunsEngine.h"

class Engine
{
public:
	Engine(RunsEngine *ref);
	virtual ~Engine() {};
	
	virtual void start();
	virtual void run() = 0;
	
	void setStepSize(float size)
	{
		_step = size;
	}

	float bestScore()
	{
		return _bestScore;
	}
	
	const bool &improved() const
	{
		return _improved;
	}
	
	const std::vector<float> &bestResult() const
	{
		return _bestResult;
	}
	
	void setMaxRuns(int maxRuns)
	{
		_maxRuns = maxRuns;
	}
	
	void setVerbose(bool verb)
	{
		_verbose = verb;
	}

	void preRun();
private:
	struct TicketScore
	{
		std::vector<float> vals;
		float score = FLT_MAX;
		bool received = false;
	};
protected:

	int sendJob(const std::vector<float> &all);
	float findBestScore();
	
	void getResults();
	void grabGradients(float *g, const float *x);
	bool getOneResult();
	
	const std::vector<float> &current() const
	{
		return _current;
	}
	
	void currentScore();
	
	void clearResults()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_scores.clear();
	}
	
	void setCurrent(const std::vector<float> &chosen)
	{
		_current = chosen;
	}
	
	std::vector<float> difference_from(std::vector<float> &other);
	void add_current_to(std::vector<float> &other);
	void add_to(std::vector<float> &other, const std::vector<float> &add);
	
	RunsEngine *const &ref()
	{
		return _ref;
	}
	
	int &n()
	{
		return _n;
	}

	bool _improved = false;
	float _step = 1.0;
	int _maxRuns = 500;
	std::map<int, TicketScore> _scores;
	std::mutex _mutex;
	std::vector<float> _current, _bestResult;
	float _bestScore = FLT_MAX;
private:
	void postRun();
	void trueGradients(float *g, const float *x);
	void estimateGradients(float *g, const float *x);

	RunsEngine *_ref = nullptr;
	bool _verbose = false;

	int _n = 0;
	float _startScore = FLT_MAX;
	float _currentScore = FLT_MAX;
	float _endScore = FLT_MAX;
};

#endif
