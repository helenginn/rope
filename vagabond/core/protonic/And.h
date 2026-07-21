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

#ifndef __vagabond__And__
#define __vagabond__And__

#include "hnet.h"
#include "ConstraintBase.h"

namespace hnet
{
template <typename Connector, typename Value>
struct And : public ConstraintBase
{
public:
	And(Connector &left, Connector &right, Connector &result,
	    const Value &expectation)
	: _left(left), _right(right), _result(result), _expectation(expectation)
	{
		prep_constraints_and_forgets(this, {&left, &right, &result});
	}

	std::string desc()
	{
		return "AND operator on \"" + _left.desc() + "\", \"" + _right.desc() 
		+ "\"";
	}

	bool value_is_not_expected(const Value &val)
	{
		Value not_expectation = (Value)(~_expectation);
		return ((val & _expectation) == 0 && (val & not_expectation));
	}

	bool value_is_not_just_expected(const Value &val)
	{
		Value not_expectation = (Value)(~_expectation);
		return ((val & _expectation) && (val & not_expectation));
	}

	bool value_is_expected_and_nothing_else(const Value &val)
	{
		Value not_expectation = (Value)(~_expectation);
		return ((val & _expectation) && (val & not_expectation) == 0);
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		Value anything;
		init_unassigned(anything);
		Value not_expectation = (Value)(anything & ~_expectation);

		Value forLeft = _left.value();
		Value forRight = _right.value();

		if (value_is_expected_and_nothing_else(forLeft) &&
		    value_is_expected_and_nothing_else(forRight))
		{
			assign(_result, _expectation);
		}

		if (value_is_not_expected(_result.value()) &&
		    value_is_not_expected(forRight) && // exist
		    value_is_not_just_expected(forLeft)) // proton
		{
			assign(_left, not_expectation);
		}

		return assign.okay();
	}
private:
	Connector &_left;
	Connector &_right;
	Connector &_result;
	Value _expectation;

};

typedef And<ExistenceConnector, Existence::Values> AndExistence;
};


#endif
