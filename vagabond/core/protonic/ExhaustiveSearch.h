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

#ifndef __vagabond__BruteForce__
#define __vagabond__BruteForce__

#include <vagabond/utils/OpSet.h>
#include <sstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "Probe.h"
//#include "Network.h"
#include "Guilt.h"
#include "ProbeResult.h"
#include "IteratedProbe.h"

class Network;
class CertainStates;

class ExhaustiveSearch
{
public:
	ExhaustiveSearch(const OpSet<Probe *> &interesting, 
	                 const OpSet<Probe *> &wider);

	void search();
	
	size_t probe_count()
	{
		return _iterations.size();
	}
	
	void setup();
	bool next();
	void cleanup();

	CertainStates *const &states() const
	{
		return _states;
	}

	/** checked once per next() call in search(), so a subnetwork's
	 * combinatorial enumeration can be interrupted mid-way rather than
	 * only between subnetworks - shared, not owned, so cancelling from a
	 * button click on the main thread never touches this object's
	 * lifetime, which is controlled by the worker thread running
	 * search(). */
	void setCancelFlag(std::shared_ptr<std::atomic<bool>> flag)
	{
		_cancel = flag;
	}
	
	virtual void tick()
	{
		std::cout << "Clicked" << std::endl;
		_cv.notify_one();
	}

	float memory_use() // MB
	{
		return _total * (float)sizeof(OneProbe) / (float)1000000.f;
	}
private:
	GetScore score_wider_clique();

	typedef std::vector<unsigned int> Config;
	OpSet<Config> _configs;

	int _total = 0;
	std::vector<ProbeResult> _results;
	CertainStates *_states{};
	std::list<IteratedProbe *> _iterations;
	OpSet<Probe *> _all;
	OpSet<Probe *> _wider;
	int _certain{0};
	int _counter{0};
	std::mutex _m;
	std::condition_variable _cv;
	GuiltVersion _gv{0};
	std::shared_ptr<std::atomic<bool>> _cancel;
};

#endif
