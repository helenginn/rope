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

#ifndef __vagabond__Coordinated_Helpers__
#define __vagabond__Coordinated_Helpers__

#include <vagabond/utils/OpSet.h>
#include "alignment.h"

namespace hnet
{

// small free-function helpers shared by more than one of the
// Coordinated_*.cpp files this class's implementation is split across
// (see that split's own note in Coordinated_Core.cpp) - kept here once
// instead of duplicated per file. Defined in Coordinated_Helpers.cpp.
// ABPair/PairSet/CovPairSet/AcceptableGroup (alignment.h) all live in
// namespace hnet, so these declarations must too - relying on a caller's
// own `using namespace hnet;` already being in effect before the
// #include (true of every Coordinated_*.cpp, since they all have one in
// their shared preamble, but not guaranteed for every future includer)
// would be fragile.

// makeHydrogenAtom() tags each auto-generated placeholder hydrogen with
// "H!" + a disambiguating index (H!0, H!1, ...) rather than the exact
// name "H!", so anything checking for a placeholder hydrogen must check
// the prefix rather than an exact match.
bool is_placeholder_hydrogen_name(const std::string &name);

OpSet<ABPair> turn_into_pairset(const CovPairSet &set);

// we need to make a custom equality function as we need to ignore
// the Atom; the fake hydrogens would otherwise always register as
// different.
bool are_equivalent(const PairSet &a_all, const PairSet &b_all);
bool are_equivalent(const AcceptableGroup &a_all, const AcceptableGroup &b_all);

void add_unique_to_set(OpSet<AcceptableGroup> &dest,
                       const AcceptableGroup &newest);

OpSet<PairSet> convert_pair_set_to_all_relationships(const PairSet &start);

}

#endif
