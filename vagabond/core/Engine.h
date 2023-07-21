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

#include <vector>
#include <map>
#include <cstddef>
#include <cfloat>
#include <functional>

class RunsEngine
{
public:
	virtual ~RunsEngine() {};
	virtual size_t parameterCount() = 0;
	virtual int sendJob(const std::vector<float> &all) = 0;

	virtual float getResult(int *job_id)
	{
		if (_scores.size() == 0)
		{
			*job_id = -1;
			return 0;
		}

		*job_id = _scores.begin()->first;
		float result = _scores.begin()->second;
		_scores.erase(_scores.begin());

		return result;
	}

	void resetTickets()
	{
		_ticket = 0;
		_scores.clear();
	}
	
	bool returnsGradients()
	{
		return (!!_gradient);
	}
protected:
	int getNextTicket()
	{
		return ++_ticket;
	}
	
	const int &getLastTicket() const
	{
		return _ticket;
	}
	
	void setGradientFunction(const std::function<std::vector<float>(int)>
	                         &gradient)
	{
		_gradient = gradient;
	}
	
	void setScoreForTicket(int ticket, double score)
	{
		_scores[ticket] = score;
	}
private:
	int _ticket = 0;
	std::map<int, double> _scores;
	std::function<std::vector<float>(int)> _gradient{};
};

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
private:
	struct TicketScore
	{
		std::vector<float> vals;
		float score = FLT_MAX;
		bool received = false;
	};
protected:

	int sendJob(const std::vector<float> &all);
	std::vector<float> findBestResult(float *score);
	
	void getResults();
	void grabGradients(float *g, const float *x);
	
	const std::vector<float> &current() const
	{
		return _current;
	}
	
	void currentScore();
	
	void clearResults()
	{
		_scores.clear();
	}
	
	void setCurrent(const std::vector<float> &chosen)
	{
		_current = chosen;
	}
	
	std::vector<float> difference_from(std::vector<float> &other);
	void add_current_to(std::vector<float> &other);
	void add_to(std::vector<float> &other, const std::vector<float> &add);
	
	int n()
	{
		return _n;
	}

	bool _improved = false;
	float _step = 1.0;
	int _maxRuns = 500;
	std::map<int, TicketScore> _scores;
private:
	void trueGradients(float *g, const float *x);
	void estimateGradients(float *g, const float *x);

	RunsEngine *_ref = nullptr;
	bool _verbose = false;

	std::vector<float> _current, _bestResult;
	int _n = 0;
	float _bestScore = FLT_MAX;
	float _startScore = FLT_MAX;
	float _currentScore = FLT_MAX;
	float _endScore = FLT_MAX;
};

#endif
