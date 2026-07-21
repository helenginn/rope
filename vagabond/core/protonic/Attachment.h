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

#ifndef __vagabond__Attachment__
#define __vagabond__Attachment__

#include "hnet.h"

namespace hnet
{
enum CountType
{
	Certain,
	Maybe,
	Not,
};

inline std::ostream &operator<<(std::ostream &ss, const CountType &ct)
{
	switch (ct)
	{
		case Certain:
		ss << "Certain"; break;
		case Maybe:
		ss << "Maybe"; break;
		case Not:
		ss << "Not"; break;
		default:
		ss << "WeirdCountType"; break;
	}
	return ss;
}

struct Attachment
{
	Attachment(BondConnector &bond, ExistenceConnector &exist) :
	_bond(bond), _existence(exist)
	{

	}
	
	template <class MakeAssign>
	bool inform(const Bond::Values &tell, MakeAssign &assign,
	            CountType certainty)
	{
		if (certainty == Maybe && _existence.value() == Existence::Unassigned)
		{
			if (assign(_bond, tell)) return true;
		}
		else if (certainty == Certain)
		{
			if (!(tell & Bond::Broken))
			{
				if (assign(_existence, Existence::Present))
				{
					return true;
				}

				if (assign(_bond, tell))
				{
					return true;
				}
			}
			else if (tell != Bond::Broken)
			{
				if (assign(_bond, tell))
				{
					return true;
				}
			}
			else if (tell == Bond::Broken)
			{
				if (_existence.value() & Existence::Present)
				{
					if (assign(_bond, tell)) return true;
				}
			}
		}
		else if (certainty == Not)
		{
			if (tell == Bond::Broken)
			{
				bool changed = false;
				changed |= assign(_existence, Existence::Present);
				changed |= assign(_bond, (Bond::Values)~tell);
				if (changed) return true;
			}
			else if (!(tell & Bond::Broken))
			{
				if (assign(_bond, (Bond::Values)~tell)) return true;
			}
		}
		return false;
	}

	Existence::Values existence_of(ExistenceConnector *const &exist)
	{
		Existence::Values existence = Existence::Present;
		if (exist)
		{
			existence = exist->value();
		}
		return existence;
	}
	
	bool value_is_not_purely_requested(const Bond::Values &val,
	                                   const Bond::Values &request)
	{
		Bond::Values not_request = (Bond::Values)(Bond::Unassigned & ~request);
		bool is_request = val & request;
		bool other = ((val & not_request) != Bond::Contradiction);
		return is_request && other;
	}
	
	bool value_is_requested_and_nothing_else(const Bond::Values &val, 
	                                         const Bond::Values &request)
	{
		Bond::Values not_request = (Bond::Values)(Bond::Unassigned & ~request);
		return ((val & request) && (val & not_request) == Bond::Contradiction);
	}

	CountType type_of_bond(const Bond::Values &request)
	{
		Existence::Values existence = existence_of(&_existence);
		Bond::Values obj = _bond.value();
		
		if (existence == Existence::Absent)
		{
			return Not;
		}
		else if (existence == Existence::Unassigned)
		{
			if (obj & request)
			{
				return Maybe;
			}
			else
			{
				return Not;
			}
		}
		else
		{
			if (value_is_not_purely_requested(obj, request))
			{
				return Maybe;
			}
			else if (value_is_requested_and_nothing_else(obj, request))
			{
				return Certain;
			}
			else
			{
				return Not;
			}
		}
	}

	BondConnector &_bond;
	ExistenceConnector &_existence;
};
};

#endif
