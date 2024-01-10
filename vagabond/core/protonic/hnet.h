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

#ifndef __vagabond__hnet__
#define __vagabond__hnet__

#include <iostream>
#include <variant>

namespace hnet
{
/*  definitions for bonds which may or may not involve a hydrogen */
namespace Bond
{
	enum Values
	{
		Contradiction = 0x00000000,
		Absent        = 0x00000001,
		Weak          = 0x00000010,
		Strong        = 0x00000100,
		Present       = 0x00000110,
		Unassigned    = 0x00000111,
	};
};

/*  definitions for atoms which either are hydrogen or absent */
namespace Hydrogen
{
enum Values
{
	Contradiction = 0x00000000,
	Absent        = 0x00000001,
	Present       = 0x00000010,
	Unassigned    = 0x00000011,
};
};

/*  definitions for atoms which are heavier than hydrogen */
namespace Atom
{
enum Values
{
	Contradiction = 0x00000000,
	Oxygen        = 0x00000001,
	Nitrogen      = 0x00000010,
	Unassigned    = 0x00000011,
};
};

/* universal functions to determine contradictory values 
 * for use in templated constraints */

inline bool is_contradictory(const Bond::Values &val)
{
	return (val == Bond::Contradiction);
}

inline bool is_contradictory(const Atom::Values &val)
{
	return (val == Atom::Contradiction);
}

inline bool is_contradictory(const Hydrogen::Values &val)
{
	return (val == Hydrogen::Contradiction);
}

inline void init_unassigned(Atom::Values &val)
{
	val = Atom::Unassigned;
}

inline void init_unassigned(Bond::Values &val)
{
	val = Bond::Unassigned;
}

inline void init_unassigned(Hydrogen::Values &val)
{
	val = Hydrogen::Unassigned;
}

/* function signatures */

typedef std::function<bool()> Checker;

/* write descriptions of states to stream */

inline std::ostream &operator<<(std::ostream &ss, const Hydrogen::Values &v)
{
	switch (v)
	{
		case Hydrogen::Contradiction:
		ss << std::string("Contradiction!");
		break;

		case Hydrogen::Absent:
		ss << std::string("Absent");
		break;

		case Hydrogen::Present:
		ss << std::string("Present");
		break;

		case Hydrogen::Unassigned:
		ss << std::string("Unassigned");
		break;

		default:
		break;
	}

	return ss;
}

inline std::ostream &operator<<(std::ostream &ss, const Bond::Values &v)
{
	switch (v)
	{
		case Bond::Contradiction:
		ss << std::string("Contradiction!");
		break;

		case Bond::Absent:
		ss << std::string("Absent");
		break;

		case Bond::Weak:
		ss << std::string("Weak");
		break;

		case Bond::Strong:
		ss << std::string("Strong");
		break;

		case Bond::Present:
		ss << std::string("Present");
		break;

		case Bond::Unassigned:
		ss << std::string("Unassigned");
		break;

		default:
		break;
	}

	return ss;
}

};

#endif
