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
#include "Network.h"
#include "ProbeResult.h"

class Decree;
class Network;

class IteratedProbe
{
public:
	virtual bool operator()(std::condition_variable &cv, std::mutex &m) = 0;
	virtual bool done() = 0;
	virtual bool is_certain() = 0;
	virtual int num() = 0;
	virtual void reset(std::condition_variable &cv, std::mutex &m) = 0;
	
	virtual std::string desc() = 0;
	virtual unsigned int value_as_int() = 0;
	virtual ~IteratedProbe() {}
	
	virtual Probe *const &probe() const = 0;
	virtual std::string type() = 0;
};

template <class Connector, class HValue>
class IterateDecree : public IteratedProbe
{
public:
	IterateDecree(Network &network, Probe *probe, 
	              Connector &connector, hnet::ExistenceConnector &exist,
	              const std::vector<HValue> &values,
	              const std::string &add)
	: _probe(probe), _connector(connector), _exist(exist), _values(values), _network(network), _add(add)
	{
		std::ostringstream msg;
		msg << "DECLARE " << probe->desc() << " (" << add << ")";
		msg << " to be one of";
		
		for (HValue &value : _values)
		{
			msg << " " << value << ",";
		}
		_msg = msg.str();
		_msg.pop_back();
	}

	virtual unsigned int value_as_int()
	{
		if (_exist.value() == hnet::Existence::Absent)
		{
			return _exist.value();
		}
		return (unsigned int)_connector.value();
	}
	
	void forget_last_decree(std::condition_variable &cv, std::mutex &m)
	{
		if (_lastDecree)
		{
//			std::cout << "FORGET " << _connector.desc() << std::endl;
			_connector.forget(_lastDecree);
			_lastDecree = nullptr;
		}
	}
	
	virtual std::string desc()
	{
		return _msg;
	}

	virtual bool operator()(std::condition_variable &cv, std::mutex &m)
	{
		forget_last_decree(cv, m);
		HValue value = _values[_num];

		while (true)
		{
			_num++;

			if (done())
			{
				return true;
			}

			if (is_certain())
			{
				continue;
			}

			value = _values[_num];

			if (_connector.value() & value)
			{
				break;
			}
		}

		_lastDecree = _network.newDecree(_msg);
		
//		std::cout << _msg << " (" << _num + 1 << " / " << _values.size() << 
//		")" << std::endl;

//		std::cout << "READY TO DECLARE SOMETHING" << std::endl;
		{
//			std::unique_lock lk(m);
//			cv.wait(lk);
		}
		bool okay = _connector.assign_value(value, _lastDecree, _lastDecree);
//		std::cout << "... result: " << (okay ? "good" : "bad, must forget") 
//		<< std::endl;
		if (!okay)
		{
			forget_last_decree(cv, m);
		}
		return okay;
	}

	virtual bool is_certain()
	{
		return (_exist.value() == hnet::Existence::Absent ||
		        _connector.is_certain());
	}
	
	virtual int num()
	{
		return _num;
	}
	
	virtual bool done()
	{
		bool done = (_num >= (int)_values.size());
		return done;
	}
	
	virtual void reset(std::condition_variable &cv, std::mutex &m)
	{
		forget_last_decree(cv, m);
		_lastDecree = nullptr;
		_num = -1;
	}
	
	Probe *const &probe() const
	{
		return _probe;
	}
	
	std::string type()
	{
		return _add;
	}
private:
	Probe *_probe{};
	Connector &_connector;
	hnet::ExistenceConnector &_exist;
	std::vector<HValue> _values{};
	std::string _msg;
	Decree *_lastDecree{};
	Network &_network;
	std::string _add;
	int _num{-1};
};

class ExhaustiveSearch
{
public:
	ExhaustiveSearch(const OpSet<Probe *> &interesting, 
	                 const OpSet<Probe *> &wider, Network &network);

	void search();
	
	size_t probe_count()
	{
		return _iterations.size();
	}
	
	void setup();
	bool next();
	void cleanup();
	
	const std::vector<ProbeResult> &results() const
	{
		return _results;
	}
	
	virtual void tick()
	{
		std::cout << "Clicked" << std::endl;
		_cv.notify_one();
	}
private:
	float score_wider_clique();

	typedef std::vector<unsigned int> Config;
	OpSet<Config> _configs;
	std::map<Config, float> _scores;

	std::vector<ProbeResult> _results;
	std::list<IteratedProbe *> _iterations;
	OpSet<Probe *> _all;
	OpSet<Probe *> _wider;
	int _certain{0};
	int _counter{0};
	Network &_network;
	std::mutex _m;
	std::condition_variable _cv;
};

#endif
