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


#ifndef __vagabond__Constraint__
#define __vagabond__Constraint__

#include <variant>
#include <functional>
#include "Connector.h"
#include "HydrogenBond.h"
#include "EitherOrBond.h"
#include "BondAdder.h"
#include "CountAdder.h"
#include "Limit.h"

namespace hnet
{
/* simple constant class to impose a value (AND) on a connector */
template <class Connector, class Value>
struct Constant
{
	Constant(Connector &connector, const Value &constant) :
	_connector(connector), _constant(constant)
	{
		/* add this object's constraint check function to the connector */
		auto self_check = [this](void *prev) { return check(prev); };
		_connector.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };
		_connector.add_forget(forget_me);

		if (!check(this))
		{
			_connector.pop_last_check(this);

			throw std::runtime_error("New constraint immediately "\
			                         "failed validation check");
		}
	}
	
	void forget(void *blame)
	{
		_connector.forget(blame);
	}
	
	bool check(void *previous)
	{
		Value candidate = Value(_connector.value() & _constant);
		if (is_contradictory(candidate))
		{
			return false;
		}
		else
		{
			return _connector.assign_value(_constant, this, previous);
		}
	}
	
	Connector &_connector;
	Value _constant;
};

/* simple typedefs */
typedef Constant<AtomConnector, Atom::Values> AtomConstant;
typedef Constant<BondConnector, Bond::Values> BondConstant;
typedef Constant<CountConnector, Count::Values> CountConstant;

/* union to store created constraints in a list */
struct AnyConstraint
{
	enum Type
	{
		Count, Atom, Bond, HBond, StrongAdd, CountAdd, WeakAdd, PresentAdd, 
		AbsentAdd, EiOrBond, Min, Max
	};
	
	AnyConstraint(MaxLimit *const &constraint)
	{
		_type = Max;
		_ptr = constraint;
	}
	
	AnyConstraint(MinLimit *const &constraint)
	{
		_type = Min;
		_ptr = constraint;
	}
	
	AnyConstraint(CountConstant *const &constraint)
	{
		_type = Count;
		_ptr = constraint;
	}
	
	AnyConstraint(AtomConstant *const &constraint)
	{
		_type = Atom;
		_ptr = constraint;
	}
	
	AnyConstraint(BondConstant *const &constraint)
	{
		_type = Bond;
		_ptr = constraint;
	}
	
	AnyConstraint(HydrogenBond *const &constraint)
	{
		_type = HBond;
		_ptr = constraint;
	}
	
	AnyConstraint(EitherOrBond *const &constraint)
	{
		_type = EiOrBond;
		_ptr = constraint;
	}
	
	AnyConstraint(WeakAdder *const &constraint)
	{
		_type = WeakAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(AbsentAdder *const &constraint)
	{
		_type = AbsentAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(StrongAdder *const &constraint)
	{
		_type = StrongAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(CountAdder *const &constraint)
	{
		_type = CountAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(PresentAdder *const &constraint)
	{
		_type = PresentAdd;
		_ptr = constraint;
	}
	
	void destroy()
	{
		switch (_type)
		{
			case Count:
			delete static_cast<CountConstant *>(_ptr); break;

			case Atom:
			delete static_cast<AtomConstant *>(_ptr); break;

			case Bond:
			delete static_cast<BondConstant *>(_ptr); break;

			case Min:
			delete static_cast<MinLimit *>(_ptr); break;

			case Max:
			delete static_cast<MaxLimit *>(_ptr); break;

			case WeakAdd:
			delete static_cast<WeakAdder *>(_ptr); break;

			case StrongAdd:
			delete static_cast<StrongAdder *>(_ptr); break;

			case PresentAdd:
			delete static_cast<PresentAdder *>(_ptr); break;

			case AbsentAdd:
			delete static_cast<AbsentAdder *>(_ptr); break;

			case CountAdd:
			delete static_cast<CountAdder *>(_ptr); break;

			case HBond:
			delete static_cast<HydrogenConnector *>(_ptr); break;
			
			case EiOrBond:
			delete static_cast<EitherOrBond *>(_ptr); break;

			default: break;
		}
	}
	
	Type _type;
	void *_ptr;
};

};

#endif
