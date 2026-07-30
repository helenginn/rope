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

#ifndef __vagabond__ResidueRange__
#define __vagabond__ResidueRange__

#include <string>
#include <vector>

/** one comma-separated "bit" of a residue range plan, e.g. "157",
 * "109-111" or "A157" - a single residue has begin == end */
struct ResidueRangeToken
{
	std::string chain;  // "" if the plan never specified one
	int begin;
	int end;
};

/** parses a comma-separated list of residue numbers/ranges, each
 * optionally prefixed with a chain letter (e.g. "A157,253,109-111,B145"),
 * a chain prefix carrying over to subsequent bits until another is given.
 * A bit with no chain prefix and none established yet is left with
 * chain == "" - resolving that is the caller's job, since this parser has
 * no model to default against.
 *
 * Returns false and fills `error` on syntax error (too many dashes in a
 * bit, or end < begin). Tokens already parsed before a failure are kept,
 * so a caller can still report partial progress, matching the behaviour
 * this was extracted from (BallAndStickOptions::addResidueRange). */
bool parseResidueRanges(const std::string &plan,
                        std::vector<ResidueRangeToken> &tokens,
                        std::string &error);

#endif
