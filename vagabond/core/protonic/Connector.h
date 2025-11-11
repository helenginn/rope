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

#ifndef __vagabond__Connector__
#define __vagabond__Connector__

#include "hnet.h"
#include "Conditions.h"
#include <functional>
#include <list>
#include <sstream>

namespace hnet
{
struct ConnectBase
{
public:
	virtual bool forget(void *blame) = 0;
	virtual ~ConnectBase() {}

	/* list of attached constraint-checking functions */
	std::list<Checker> _checks;
	std::list<Forget> _forgets;
	
	/* add a constraint check function to the list of functions to run on
	 * an update of value */
	void add_constraint_check(const Checker &checker)
	{
		_checks.push_back(checker);
	}
	
	/* add a forget routine called from a constraint */
	void add_forget(const Forget &forget)
	{
		_forgets.push_back(forget);
	}
	
	void pop_last_check(void *blame)
	{
		OpSet<void *> guilts = Guilt::guilt().rollBackBefore(blame);
		forget(blame);

		_checks.pop_back();
		_forgets.pop_back();
	}
	
	inline static std::ostringstream _out{};
	inline static bool _silent{false};

	static std::ostringstream &my_out()
	{
		return _out;
	}
	
	static std::ostream &out()
	{
		return (ConnectBase::_silent ? _out : std::cout);
	}
};

template <typename Value>
struct Connector : public ConnectBase
{
	Connector()
	{
		init_unassigned(_value);
		_default = _value;
	}
	
	~Connector()
	{

	}

	Connector(const Value &init)
	{
		_value = init;
		_default = _value;
	}

	
	void report()
	{
		std::cout << "===============" << std::endl;
		std::cout << "Report for " << this << std::endl;
		_conditions.report_conditions();
	}
	
	void setDesc(const std::string &desc)
	{
		_desc = desc;
	}
	
	const std::string &desc() const
	{
		return _desc;
	}

	/* returns true if changed */
	bool assign_value_without_checking(const Value &value, void *informant,
	                                   void *blame)
	{
		if (_conditions.from_informant_and_blame(informant, blame) == value)
		{
			// if there's no change in value then we end the propagation
			return false;
		}
		
		Value before = _conditions.belief();
		_conditions.apply_condition(informant, blame, value);
		Value after = _conditions.belief();

		bool changed = (before != after);
		if (changed && _desc.length())
		{
			ConnectBase::out() << "CONNECTOR: \"" << *this << "\" was " 
			<< before << ", before applying condition " << value << 
			" resulting in " << after << std::endl;
		}
		
		return changed;
	}

	bool forget(void *blame)
	{
		OpSet<void *> guilts = Guilt::guilt().rollBackBefore(blame);
		return forget(guilts);
	}

	bool forget(OpSet<void *> &guilts)
	{
		Value before = _conditions.belief();
		int grand_total = _conditions.size();

		int total = _conditions.remove_conditions_with_blame(guilts);

		if (total == 0)
		{
			return false;
		}

		if (_update)
		{
			_update();
		}

		if (false)
		{
			std::cout << "ABSOLVE: " << desc() << " forgetting: " << guilts.size() << 
			" (" << grand_total << " - " << total << " = " << _conditions.size() << " conditions)" << std::endl;
			std::cout << "\t" << desc() << " was " << before << " now " << _conditions.belief() << std::endl;
		}

		/* if we reassign a new value, we must invoke forget cascade */
		for (Forget &forget_cascade : _forgets)
		{
			forget_cascade(guilts);
		}
		
		return true;
	}
		
	bool check_all(void *blame)
	{
		/* if we reassign a new value, we recalculate checks */
		for (Checker &checker : _checks)
		{
			if (!checker(blame))
			{
				std::cout << "That was a bad check, abort" << std::endl;
				return false;
			}
		}

		if (is_contradictory(value()))
		{
			return false;
		}
		
		
		return true;
	}
	
	bool is_certain()
	{
		Value belief = _conditions.belief();

		return certain(belief);
	}
	
	bool assign_value(const Value &value, void *informant, void *blame)
	{
		if (assign_value_without_checking(value, informant, blame))
		{
			if (_update)
			{
				_update();
			}

			// handles next assignment
			bool result = check_all(blame);
			
			if (!result && !ConnectBase::_silent)
			{
				std::cout << ConnectBase::my_out().str();
			}

			return result;
		}

		return true;
	}
	
	void set_update(const UpdateProbe &update)
	{
		_update = update;
	}

	Value value(bool *access = nullptr)
	{
		return _conditions.belief(access);
	}
	
	std::vector<Value> values()
	{
		Value as_one = _conditions.belief();
		return split_into_options(as_one);
	}

	UpdateProbe _update;

	/* working value associated with this connector */
	Value _value = {};
	Value _default = {};
	
	Conditions<Value> _conditions;
	
	std::string _desc{};
	bool _user = false;
};

template <typename Value>
inline std::ostream &operator<<(std::ostream &ss, const Connector<Value> &c)
	{
		if (c._desc.length())
		{
			ss << c._desc;
		}
		else ss << "(unnamed)";
		return ss;
	}

typedef Connector<Atom::Values> AtomConnector;
typedef Connector<Bond::Values> BondConnector;
typedef Connector<Existence::Values> ExistenceConnector;
typedef Connector<Count::Values> CountConnector;

/* union to store created connectors in a list */
struct AnyConnector
{
	enum Type
	{
		Atom, Bond, Existence, Count
	};
	
	AnyConnector(BondConnector *const &connector)
	{
		_type = Bond;
		_ptr = connector;
	}
	
	AnyConnector(CountConnector *const &connector)
	{
		_type = Count;
		_ptr = connector;
	}
	
	AnyConnector(AtomConnector *const &connector)
	{
		_type = Atom;
		_ptr = connector;
	}
	
	AnyConnector(ExistenceConnector *const &connector)
	{
		_type = Existence;
		_ptr = connector;
	}
	
	void destroy()
	{
		switch (_type)
		{
			case Atom:
			delete static_cast<AtomConnector *>(_ptr);
			break;

			case Bond:
			delete static_cast<BondConnector *>(_ptr);
			break;

			case Count:
			delete static_cast<CountConnector *>(_ptr);
			break;

			case Existence:
			delete static_cast<ExistenceConnector *>(_ptr);
			break;
			
			default: break;
		}
	}
	
	Type _type;
	void *_ptr;
};

template <typename Type> struct Connector;

template <class Me>
struct make_assign_and_say
{
	make_assign_and_say(Me *me, void *previous)
	: _me(me), _prev(previous) {}

	template <typename Type>
	bool operator()(Connector<Type> &which, const Type &what,
	                const std::string &reason = "")
	{
		ConnectBase::my_out() = {};
		Type before = which.value();
		_okay &= which.assign_value(what, _me, _prev);
		Type after = which.value();

		if (before != after)
		{
			ConnectBase::out() << "... from CONSTRAINT: \"" << _me->desc() << 
			"\" forcing assignment of " << what << " on " << 
			which << " resulting in " << after;
			if (reason.size())
			{
				std::cout << ", due to reason: " << reason;
			}
			std::cout << std::endl;
			return true;
		}

		return false;
	}
	
	const bool &okay() const
	{
		return _okay;
	}

	Me *_me{};
	void *_prev{};
	bool _okay{true};
};

template <typename ConstraintType>
void prep_constraints_and_forgets(ConstraintType *constraint,
                                  const std::vector<ConnectBase *> &connections)
{
	auto self_check = [constraint](void *prev)
	{
		return constraint->check(prev);
	};

	auto forget_me = [constraint](OpSet<void *> &blame)
	{
		return constraint->forget(blame);
	};

	for (ConnectBase *connector : connections)
	{
		if (connector)
		{
			connector->add_constraint_check(self_check);
			connector->add_forget(forget_me);
		}
	}

	if (!constraint->check(constraint))
	{
		for (ConnectBase *connector : connections)
		{
			if (connector)
			{
				connector->pop_last_check(constraint);
			}
		}

		throw std::runtime_error("New constraint immediately "\
		                         "failed validation check");
	}
}

};

#endif
