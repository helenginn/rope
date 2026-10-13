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

#include <iostream>
#include <vector>
#include <map>
#include <cstddef>
#include <cfloat>
#include <functional>
#include <mutex>

typedef std::function<void(std::vector<float> &values)> Getter;
typedef std::function<void(const std::vector<float> &values)> Setter;

class Engine;

class RunsEngine
{
public:
	virtual ~RunsEngine() {};

	virtual float getResult(int *job_id, Engine *sender)
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);

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
		std::unique_lock<std::mutex> lock(_lock.mutex);
		_ticket = 0;
		_scores.clear();
	}
	
protected:
	int getNextTicket()
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);
		return ++_ticket;
	}
	
	const int getLastTicket() const
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);
		return _ticket;
	}
	
	float scoreForTicket(int ticket)
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);
		std::map<int, double>::const_iterator it = _scores.find(ticket);

		if (it == _scores.end())
		{
			lock.unlock();
			std::cout << "missing ticket " << ticket << std::endl;
			return FLT_MAX;
		}

		return it->second;
	}

	/** as scoreForTicket, but forgets the score once it has been collected
	 *  (for callers which collect each ticket exactly once) */
	float takeScoreForTicket(int ticket)
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);
		std::map<int, double>::iterator it = _scores.find(ticket);

		if (it == _scores.end())
		{
			lock.unlock();
			std::cout << "missing ticket " << ticket << std::endl;
			return FLT_MAX;
		}

		float score = it->second;
		_scores.erase(it);
		return score;
	}
	
	void setScoreForTicket(int ticket, double score)
	{
		std::unique_lock<std::mutex> lock(_lock.mutex);
		_scores[ticket] = score;
	}
private:
	/* Tickets are handed out and scores filed by whichever thread finishes a
	 * batch, while the engines collecting them run on other threads which
	 * hold no lock in common with it (MultiEngine only locks per key). An
	 * unguarded map being looked up during an insert can report a ticket
	 * as missing. This mutex guards _ticket and _scores only, and nothing
	 * else is called while it is held. */
	struct TicketLock
	{
		TicketLock() {}
		// copies of an engine get their own mutex
		TicketLock(const TicketLock &) {}
		TicketLock &operator=(const TicketLock &) { return *this; }

		mutable std::mutex mutex;
	};

	TicketLock _lock;
	int _ticket = 0;
	std::map<int, double> _scores;
};

#endif
