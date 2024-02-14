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
#include <vector>
#include <functional>

/** serious missing features:
 * - missing hydrogens off non-donors
 * - donatable hydrogens which are incompatible with each other
 *    - in trypsin, Ser170 and Ser167 for example
 * - alternate conformers
 */

namespace hnet
{
/*  definitions for bonds which may or may not involve a hydrogen */
namespace Bond
{
	enum Values
	{
		Contradiction =  (0),
		Absent        =  (1 << 0),
		NotAbsent     =  (1 << 1 | 1 << 2 | 1 << 3),
		AbsentOrWeak  =  (1 << 0 | 1 << 1),
		Weak          =  (1 << 1),
		NotWeak       =  (1 << 0 | 1 << 2 | 1 << 3),
		Strong        =  (1 << 2),
		NotStrong     =  (1 << 0 | 1 << 1 | 1 << 3),
		Present       =  (1 << 1 | 1 << 2),
		NotPresent    =  (1 << 0 | 1 << 3),
		Broken        =  (1 << 3),
		NotBroken     =  (1 << 0 | 1 << 1 | 1 << 2),
		Unassigned    =  (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3),
	};
};

/*  definitions for atoms which either are hydrogen or absent */
namespace Hydrogen
{
	enum Values
	{
		Contradiction = 0,
		Absent        = 1 << 0,
		Present       = 1 << 1,
		Unassigned    = (1 << 0 | 1 << 1)
	};
};

/*  definitions for atoms which are heavier than hydrogen */
namespace Atom
{
	enum Values
	{
		Contradiction = 0,
		Oxygen        = 1 << 0,
		Nitrogen      = 1 << 1,
		Sulphur       = 1 << 2,
		Unassigned    = (1 << 0 | 1 << 1 | 1 << 2),
	};
};

/*  definitions for atoms which are heavier than hydrogen */
namespace Count
{
	enum Values
	{
		Contradiction = 0,
		Zero          = 1 << 0,
		One           = 1 << 1,
		Two           = 1 << 2,
		Three         = 1 << 3,
		Four          = 1 << 4,
		Five          = 1 << 5,
		Six           = 1 << 6,
		Seven         = 1 << 7,
		Eight         = 1 << 8,
		Nine          = 1 << 9,
		Ten           = 1 << 10,
		Eleven        = 1 << 11,
		Twelve        = 1 << 12,
		Thirteen      = 1 << 13,
		Fourteen      = 1 << 14,
		Fifteen       = 1 << 15,
		mOne           = 1 << 16,
		mTwo           = 1 << 17,
		mThree         = 1 << 18,
		mFour          = 1 << 19,
		mFive          = 1 << 20,
		mSix           = 1 << 21,
		mSeven         = 1 << 22,
		mEight         = 1 << 23,
		mNine          = 1 << 24,
		mTen           = 1 << 25,
		mEleven        = 1 << 27,
		mTwelve        = 1 << 28,
		mThirteen      = 1 << 29,
		mFourteen      = 1 << 30,
		mFifteen       = 1 << 31,
		OneOrZero      = (1 << 0 | 1 << 1),
		mOneOrZero     = (1 << 0 | 1 << 16),
		TwoToZero      = (1 << 0 | 1 << 1 | 1 << 2),
		ThreeToZero    = (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3),
		ZeroOrMore     = (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 
		                  1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8 | 1 << 9
		                  | 1 << 10 | 1 << 11 | 1 << 12 | 1 << 13 | 1 << 14 
		                  | 1 << 15),
		MoreThanZero   = (         1 << 1 | 1 << 2 | 1 << 3 | 
		                  1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8 | 1 << 9
		                  | 1 << 10 | 1 << 11 | 1 << 12 | 1 << 13 | 1 << 14 
		                  | 1 << 15),
		Unassigned    = ~(0)
	};
};

inline std::string to_string(int val)
{
	std::string str = "0b";
	for (int j = 31; j >= 0; j--)
	{
		unsigned char byte = (val >> j) & 1;
		str += (byte ? "1" : "0");
	}
	return str;
}

template <typename Type>
bool certain(const Type &val)
{
	int total = 0;
	for (int j = 31; j >= 0; j--)
	{
		unsigned char byte = (val >> j) & 1;
		total += byte ? 1 : 0;
		
		if (total > 1)
		{
			return false;
		}
	}

	return true;

}

/* universal functions to determine unassigned values 
 * for use in templated constraints */

inline bool is_unassigned(const Bond::Values &val)
{
	return (val == Bond::Unassigned);
}

inline bool is_unassigned(const Atom::Values &val)
{
	return (val == Atom::Unassigned);
}

inline bool is_unassigned(const Hydrogen::Values &val)
{
	return (val == Hydrogen::Unassigned);
}

inline bool is_unassigned(const Count::Values &val)
{
	return (val == Count::Unassigned);
}

/* automatically set an unassigned value
 * for use in templated constraints */

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

inline void init_unassigned(Count::Values &val)
{
	val = Count::Unassigned;
}

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

inline bool is_contradictory(const Count::Values &val)
{
	return (val == Count::Contradiction);
}

/* function signatures */

typedef std::function<void()> UpdateProbe;
typedef std::function<bool(void *previous)> Checker;
typedef std::function<void(void *previous)> Forget;

/* write descriptions of states to stream */

inline std::ostream &operator<<(std::ostream &ss, const Count::Values &v)
{
	switch (v)
	{
		default:
		ss << to_string(v);
		break;
	}

	return ss;
}

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
		ss << to_string(v);
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

		case Bond::AbsentOrWeak:
		ss << std::string("AbsentOrWeak");
		break;

		case Bond::NotAbsent:
		ss << std::string("NotAbsent");
		break;

		case Bond::Weak:
		ss << std::string("Weak");
		break;

		case Bond::NotWeak:
		ss << std::string("NotWeak");
		break;

		case Bond::Strong:
		ss << std::string("Strong");
		break;

		case Bond::NotStrong:
		ss << std::string("NotStrong");
		break;

		case Bond::Present:
		ss << std::string("Present");
		break;

		case Bond::NotPresent:
		ss << std::string("NotPresent");
		break;

		case Bond::NotBroken:
		ss << std::string("NotBroken");
		break;

		case Bond::Broken:
		ss << std::string("Broken");
		break;

		case Bond::Unassigned:
		ss << std::string("Unassigned");
		break;

		default:
		ss << to_string(v);
		ss << std::string("Not sure? " + std::to_string(v));
		break;
	}

	return ss;
}

/* for counts / integer conversion */
inline Count::Values values_as_count(const std::vector<int> &values)
{
	auto int_to_count = [](const int &val)
	{
		if (val >= 0)
		{
			return (Count::Values)(1 << val);
		}
		else
		{
			return (Count::Values)(1 << (-val + 15));
		}
	};
	
	Count::Values addition = Count::Contradiction;
	for (const int &value : values)
	{
		if (value > 15 || value < -15)
		{
			continue;
		}

		Count::Values isolated = int_to_count(value);
		addition = (Count::Values)(addition | isolated);
	}
	
	return addition;
};

/* for integers, but can also be unassigned or contradictory */
inline std::vector<int> possible_values(const Count::Values &count)
{
	auto count_to_int = [](const Count::Values &val)
	{
		switch (val)
		{
			case Count::Zero:
			return 0;

			case Count::One:
			return 1;

			case Count::Two:
			return 2;

			case Count::Three:
			return 3;

			case Count::Four:
			return 4;

			case Count::Five:
			return 5;

			case Count::Six:
			return 6;

			case Count::Seven:
			return 7;

			case Count::Eight:
			return 8;

			case Count::Nine:
			return 9;

			case Count::Ten:
			return 10;

			case Count::Eleven:
			return 11;

			case Count::Twelve:
			return 12;

			case Count::Thirteen:
			return 13;

			case Count::Fourteen:
			return 14;

			case Count::Fifteen:
			return 15;

			case Count::mOne:
			return -1;

			case Count::mTwo:
			return -2;

			case Count::mThree:
			return -3;

			case Count::mFour:
			return -4;

			case Count::mFive:
			return -5;

			case Count::mSix:
			return -6;

			case Count::mSeven:
			return -7;

			case Count::mEight:
			return -8;

			case Count::mNine:
			return -9;

			case Count::mTen:
			return -10;

			case Count::mEleven:
			return -11;

			case Count::mTwelve:
			return -12;

			case Count::mThirteen:
			return -13;

			case Count::mFourteen:
			return -14;

			case Count::mFifteen:
			return -15;

			default:
			throw std::runtime_error("straightforward conversion to integer"\
			                         " impossible, val = " + std::to_string(val));
		}
	};
	
	std::vector<int> results;
	for (int i = 0; i < 32; i++)
	{
		Count::Values check = (Count::Values)(1 << i);
		Count::Values isolated = (Count::Values)(count & check);

		if (isolated != Count::Contradiction)
		{
			results.push_back(count_to_int(isolated));
		}
	}
	
	return results;
};

template <typename Type>
std::vector<Type> split_into_options(const Type &belief)
{
	std::vector<Type> results;
	for (int i = 0; i < 32; i++)
	{
		Type check = (Type)(1 << i);
		Type isolated = (Type)(belief & check);

		if (!is_contradictory(isolated))
		{
			results.push_back(isolated);
		}
	}
	
	return results;
}

};

#endif
