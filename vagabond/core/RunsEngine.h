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

#ifndef __vagabond__RunsEngine__
#define __vagabond__RunsEngine__

#include <vector>
#include <map>
#include <cstddef>
#include <cfloat>
#include <functional>

typedef std::function<void(std::vector<float> &values)> Getter;
typedef std::function<void(const std::vector<float> &values)> Setter;

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

#endif
