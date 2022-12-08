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
#include <cfloat>

class RunsEngine
{
public:
	virtual ~RunsEngine() {};
	virtual size_t parameterCount() = 0;
	virtual int sendJob(std::vector<float> &all) = 0;
	virtual float getResult(int *job_id) = 0;
};

class Engine
{
public:
	Engine(RunsEngine *ref);

private:
	struct TicketScore
	{
		std::vector<float> vals;
		float score = FLT_MAX;
		bool received = false;
	};
protected:
	void sendJob(std::vector<float> &all);
	
	void getResults();
	
	std::vector<float> findBestResult(float *score);
	void currentScore();
	
	void clearResults()
	{
		_scores.clear();
	}
	
	float bestScore()
	{
		return _best;
	}
	
	void setCurrent(std::vector<float> &chosen)
	{
		_current = chosen;
	}
	
	const std::vector<float> &current() const
	{
		return _current;
	}
	
	std::vector<float> difference_from(std::vector<float> &other);
	void add_current_to(std::vector<float> &other);
	void add_to(std::vector<float> &other, const std::vector<float> &add);
	
	int n()
	{
		return _n;
	}
private:
	RunsEngine *_ref = nullptr;

	std::map<int, TicketScore> _scores;
	std::vector<float> _current;
	float _best = FLT_MAX;
	int _n = 0;
};

#endif
