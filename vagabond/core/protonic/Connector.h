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
	virtual bool forget(const GuiltVersion &gv, CheckList &list) = 0;
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
	
	void pop_last_check(const GuiltVersion &gv, CheckList &list)
	{
		do_forgets(gv, list);

		_checks.pop_back();
		_forgets.pop_back();
	}

	bool check_list(const GuiltVersion &gv, CheckList &list)
	{
		/* if we reassign a new value, we recalculate checks */
		while (list.size())
		{
			void *ptr = list.back();
			list.pop_back();
			Checker *cptr = static_cast<Checker *>(ptr);
			Checker &checker = *cptr;

			bool okay = checker(gv, list);
			
			if (!okay || contradictory())
			{
				return false;
			}
		}
		
		return true;
	}

	void add_to_forget_list(const GuiltVersion &gv, CheckList &list)
	{
		for (Forget &next_forget : _forgets)
		{
			list.push_back(&next_forget);
		}
	}

	bool do_forgets(const GuiltVersion &gv, CheckList &list)
	{
		while (list.size())
		{
			void *ptr = list.front();
			list.pop_front();
			Checker *fptr = static_cast<Checker *>(ptr);
			Checker &forgetter = *fptr;

			forgetter(gv, list);
		}
		
		return true;
	}

	
	virtual bool contradictory() = 0;

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

	virtual bool contradictory()
	{
		return is_contradictory(value());
	}

	/* returns true if changed */
	bool assign_value_without_checking(const Value &value, void *informant,
	                                   const GuiltVersion &gv)
	{
		/*
		if (_conditions.from_informant_and_blame(informant, gv) == value)
		{
			// if there's no change in value then we end the propagation
			return false;
		}
		*/
		
		Value before = _conditions.belief();
		bool constriction = (before & ~value);
		if (!constriction)
		{
			return false;
		}

		_conditions.apply_condition(informant, gv, value);
		Value after = _conditions.belief();

		bool changed = (before != after);
		if (changed && _update)
		{
			_update();
		}

		if (changed && _desc.length() && !ConnectBase::_silent)
		{
			ConnectBase::out() << "CONNECTOR: \"" << *this << "\" was " 
			<< before << ", before applying condition " << value << 
			" resulting in " << after << std::endl;
		}
		
		return changed;
	}

	bool assign_value_and_check(const Value &val, const GuiltVersion &gv)
	{
		bool changed = assign_value_without_checking(val, this, gv);
		if (!changed) return true;
		if (is_contradictory(value())) return false;

		CheckList list;
		add_to_check_list(list);
		return check_list(gv, list);
	}

	void forget_all(const GuiltVersion &gv)
	{
		CheckList list;
		add_to_forget_list(gv, list);
		do_forgets(gv, list);
	}
	
	void check_all(const GuiltVersion &gv)
	{
		CheckList list;
		add_to_check_list(list);
		check_list(gv, list);
	}
	
	bool forget(const GuiltVersion &gv, CheckList &list)
	{
		int total = _conditions.remove_conditions_with_blame(gv);

		if (total == 0)
		{
			return false;
		}

		if (_update)
		{
			_update();
		}
		
		add_to_forget_list(gv, list);
		
		return true;
	}


	void add_to_check_list(CheckList &list)
	{
		/* if we reassign a new value, we recalculate checks */
		for (Checker &checker : _checks)
		{
			list.push_back(&checker);
		}
	}
	
	bool is_certain()
	{
		Value belief = _conditions.belief();

		return certain(belief);
	}
	
	bool assign_value(const Value &value, void *informant,
	                  const GuiltVersion &gv, CheckList &list)
	{
		if (assign_value_without_checking(value, informant, gv))
		{
			// handles next assignment
			add_to_check_list(list);
			
			bool okay = !is_contradictory(value);
			if (!okay && !ConnectBase::_silent)
			{
				std::cout << ConnectBase::my_out().str();
			}
			
			return okay;
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
	make_assign_and_say(Me *me, const GuiltVersion &gv, CheckList &list)
	: _me(me), _gv(gv), _list(list) {}

	template <typename Type>
	bool operator()(Connector<Type> &which, const Type &what,
	                const std::string &reason = "")
	{
		if (!ConnectBase::_silent)
		{
			ConnectBase::my_out().str("");
		}
		Type before = which.value();
		_okay &= which.assign_value(what, _me, _gv, _list);
		Type after = which.value();

		if (before != after && !ConnectBase::_silent)
		{
			ConnectBase::out() << "... from CONSTRAINT: \"" << _me->desc() << 
			"\" forcing assignment of " << what << " on " << 
			which << " resulting in " << after;
			if (reason.size())
			{
				ConnectBase::out() << ", due to reason: " << reason;
			}
			ConnectBase::out() << std::endl;
			return true;
		}

		return false;
	}
	
	const bool &okay() const
	{
		return _okay;
	}

	Me *_me{};
	GuiltVersion _gv;
	bool _okay{true};
	CheckList &_list;
};

template <typename ConstraintType>
void prep_constraints_and_forgets(ConstraintType *constraint,
                                  const std::vector<ConnectBase *> &connections)
{
	auto make_self_check = [constraint](ConnectBase *me)
	{
		return [me, constraint](const GuiltVersion &gv, CheckList &list)
		{
			bool okay = constraint->check(gv, list);
			okay |= me->contradictory();
			return okay;
		};
	};

	auto forget_connector = [constraint]()
	{
		return [constraint](const GuiltVersion &gv, CheckList &list)
		{
			constraint->forget(gv, list);
		};
	};

	for (ConnectBase *connector : connections)
	{
		if (connector)
		{
			connector->add_constraint_check(make_self_check(connector));
			connector->add_forget(forget_connector());
			constraint->to_forgets.push_back(connector);
		}
	}

	for (ConnectBase *connector : connections)
	{
		if (!connector)
		{
			continue;
		}

		CheckList list; 
		Checker job = make_self_check(connector);
		list.push_back(&job);

		GuiltVersion test = Guilt::issueNext();
		if (!connector->check_list(test, list))
		{
			for (ConnectBase *connector : connections)
			{
				if (connector)
				{
					connector->pop_last_check(test, list);
				}
			}

			throw std::runtime_error("New constraint immediately "\
			                         "failed validation check");
		}
	}
}

};

#endif
