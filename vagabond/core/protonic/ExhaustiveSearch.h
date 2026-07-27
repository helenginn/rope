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
};

#endif
