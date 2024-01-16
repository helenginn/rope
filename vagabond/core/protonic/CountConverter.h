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

#ifndef __vagabond__CountConverter__
#define __vagabond__CountConverter__

namespace hnet
{
template <class Connector, class Value>
struct Converter
{
	Converter(Connector &affected, Connector &imposer, 
	               const Value &result,
	               const Value &condition)
	: _affected(affected), _imposer(imposer), _condition(condition),
	_result(result)
	{
		/* add this object's constraint check function to the connector */
		_affected.add_constraint_check([this](){ return check(); });
		_imposer.add_constraint_check([this](){ return check(); });

		if (!check())
		{
			throw std::runtime_error("New constraint immediately "\
			                         "failed validation check");
		}
	}

	bool check()
	{
		Value v; init_unassigned(v);
		Value not_condition = (Value)(v & ~_condition);

		bool satisfies = (_imposer.value() & _condition);
		std::cout << _affected.value() << " " << _imposer.value() << " " << _condition << " " << not_condition << std::endl;

		std::cout << "Imposer has condition? : " << satisfies << std::endl;
		// also mustn't be able to not be this condition
		satisfies &= !(_imposer.value() & (~_condition & v));
		std::cout << "Imposer also is not not-condition? : " << satisfies << std::endl;
		if (satisfies)
		{
			_affected.assign_value(_result, this);
		}
		else
		{
			_affected.assign_value(v, this);
		}

		bool con = (!is_contradictory(_affected.value()));

		return con;
	}

	Connector &_affected;
	Connector &_imposer;
	Value _condition;
	Value _result;
};

typedef Converter<CountConnector, Count::Values> CountConverter;

};

#endif
