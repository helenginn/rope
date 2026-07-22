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
#include "ConstraintBase.h"
#include "And.h"
#include "OnlyOne.h"
#include "Stricter.h"
#include "Connector.h"
#include "BondAdder.h"
#include "EqualBonds.h"
#include "CountAdder.h"
#include "Attachment.h"
#include "HydrogenBond.h"
#include "SubExistence.h"
#include "BreakMatrix.h"
#include "MaxOne.h"
#include "MutualExistence.h"

/* simple constant class to impose a value (AND) on a connector */
namespace hnet
{
template <class Connector, class Value>
struct Constant : public ConstraintBase
{
	Constant(Connector &connector, const Value &constant) :
	_connector(connector), _constant(constant)
	{
		prep_constraints_and_forgets(this, {&connector});
	}
	
	std::string desc()
	{
		return "Constant for connector " + _connector.desc();
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		bool changed = true;
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
typedef Constant<CovalentConnector, Covalent::Values> CovalentConstant;

};

#endif
