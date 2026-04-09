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
#include "Limit.h"
#include "OnlyOne.h"
#include "Stricter.h"
#include "Connector.h"
#include "BondAdder.h"
#include "EqualBonds.h"
#include "CountAdder.h"
#include "HydrogenBond.h"
#include "EitherOrBond.h"
#include "SubExistence.h"
#include "BreakMatrix.h"
#include "MutualExistence.h"
//#include "LinkBondPresence.h"

namespace hnet
{
/* simple constant class to impose a value (AND) on a connector */
template <class Connector, class Value>
struct Constant
{
	Constant(Connector &connector, const Value &constant) :
	_connector(connector), _constant(constant)
	{
		prep_constraints_and_forgets(this, {&connector});
	}
	
	void forget(const GuiltVersion &gv)
	{
		_connector.forget(gv);
	}
	
	std::string desc()
	{
		return "Constant for connector " + _connector.desc();
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		assign(_connector, _constant);
		return assign.okay();
	}
	
	Connector &_connector;
	Value _constant;
};


/* simple typedefs */
typedef Constant<AtomConnector, Atom::Values> AtomConstant;
typedef Constant<BondConnector, Bond::Values> BondConstant;
typedef Constant<CountConnector, Count::Values> CountConstant;
typedef Constant<ExistenceConnector, Existence::Values> ExistenceConstant;

/* union to store created constraints in a list */
struct AnyConstraint
{
	enum Type
	{
		Count, Atom, Bond, HBond, StrongAdd, CountAdd, WeakAdd, BondedAdd, 
		LonePairAdd, NotBrokenAdd, EiOrBond, Equal, StricterCount,
		Existence, MutualExist, SubExist, OnlyOneOf, IfCountThen, 
		BreakingMatrix, StrictBond, StricterExistence,
	};
	
	AnyConstraint(hnet::Stricter<Existence::Values> *const &constraint)
	{
		_type = StricterExistence;
		_ptr = constraint;
	}
	
	AnyConstraint(StrictCount *const &constraint)
	{
		_type = StricterCount;
		_ptr = constraint;
	}
	
	AnyConstraint(OnlyOne *const &constraint)
	{
		_type = OnlyOneOf;
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
	
	AnyConstraint(SubExistence *const &constraint)
	{
		_type = SubExist;
		_ptr = constraint;
	}
	
	AnyConstraint(MutualExistence *const &constraint)
	{
		_type = MutualExist;
		_ptr = constraint;
	}
	
	AnyConstraint(ExistenceConstant *const &constraint)
	{
		_type = Existence;
		_ptr = constraint;
	}
	
	AnyConstraint(EqualBonds *const &constraint)
	{
		_type = Equal;
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
	
	AnyConstraint(NotBrokenAdder *const &constraint)
	{
		_type = NotBrokenAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(WeakAdder *const &constraint)
	{
		_type = WeakAdd;
		_ptr = constraint;
	}
	
	AnyConstraint(LonePairAdder *const &constraint)
	{
		_type = LonePairAdd;
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
	
	AnyConstraint(BreakMatrix *const &constraint)
	{
		_type = BreakingMatrix;
		_ptr = constraint;
	}
	
	AnyConstraint(StricterBond *const &constraint)
	{
		_type = StrictBond;
		_ptr = constraint;
	}
	
	AnyConstraint(BondedAdder *const &constraint)
	{
		_type = BondedAdd;
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

			case WeakAdd:
			delete static_cast<WeakAdder *>(_ptr); break;

			case StrongAdd:
			delete static_cast<StrongAdder *>(_ptr); break;

			case BondedAdd:
			delete static_cast<BondedAdder *>(_ptr); break;

			case LonePairAdd:
			delete static_cast<LonePairAdder *>(_ptr); break;

			case NotBrokenAdd:
			delete static_cast<NotBrokenAdder *>(_ptr); break;

			case CountAdd:
			delete static_cast<CountAdder *>(_ptr); break;

			case MutualExist:
			delete static_cast<MutualExistence *>(_ptr); break;

			case SubExist:
			delete static_cast<SubExistence *>(_ptr); break;

			case Existence:
			delete static_cast<ExistenceConnector *>(_ptr); break;

			case HBond:
			delete static_cast<ExistenceConnector *>(_ptr); break;
			
			case Equal:
			delete static_cast<EqualBonds *>(_ptr); break;
			
			case EiOrBond:
			delete static_cast<EitherOrBond *>(_ptr); break;
			
			case OnlyOneOf:
			delete static_cast<OnlyOne *>(_ptr); break;

			case BreakingMatrix:
			delete static_cast<BreakMatrix *>(_ptr); break;
			
			case StrictBond:
			delete static_cast<StricterBond *>(_ptr); break;
			
			case StricterExistence:
			delete static_cast<hnet::Stricter<Existence::Values> *>(_ptr); break;

			case StricterCount:
			delete static_cast<StrictCount *>(_ptr); break;

			default: break;
		}
	}
	
	Type _type;
	void *_ptr;
};

};

#endif
