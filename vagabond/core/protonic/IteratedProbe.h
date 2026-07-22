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

#ifndef __vagabond__IteratedProbe__
#define __vagabond__IteratedProbe__

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
	IterateDecree(Probe *probe, 
	              Connector &connector, 
	              hnet::ExistenceConnector &exist,
	              hnet::BondConnector *bond,
	              const std::vector<HValue> &values,
	              const std::string &add)
	: _probe(probe), _connector(connector), _exist(exist), _bond(bond),
	_values(values), _add(add)
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

	unsigned int connector_as_int()
	{
		return (unsigned int)_connector.value();
	}

	virtual unsigned int value_as_int()
	{
		if (_exist.value() == hnet::Existence::Absent)
		{
			return hnet::Existence::Absent;
		}
		if (_bond && (_bond->value() == hnet::Bond::Broken ||
		              _bond->value() == hnet::Bond::LonePair))
		{
			return hnet::Existence::Absent;
		}

		return (unsigned int)connector_as_int();
	}
	
	void forget_last_decree(std::condition_variable &cv, std::mutex &m)
	{
		if (_lastDecree >= 0)
		{
			_connector.forget_all(_lastDecree);
			_lastDecree = -1;
		}
	}
	
	virtual std::string desc()
	{
		return _msg;
	}

	virtual bool operator()(std::condition_variable &cv, std::mutex &m)
	{
		forget_last_decree(cv, m);
		HValue value = _values[_num < 0 ? 0 : _num];

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

		_lastDecree = Guilt::issueNext();
		
//		std::cout << _msg << " (" << _num + 1 << " / " << _values.size() << 
//		")" << std::endl;

//		std::cout << "READY TO DECLARE SOMETHING" << std::endl;
		{
//			std::unique_lock lk(m);
//			cv.wait(lk);
		}
//		hnet::ConnectBase::out() << "Declaring: " << desc() << std::endl;
		bool okay = _connector.assign_value_and_check(value, _lastDecree);
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
	hnet::BondConnector *_bond{};
	std::vector<HValue> _values{};
	std::string _msg;
	GuiltVersion _lastDecree{-1};
	std::string _add;
	int _num{-1};
};

#endif
