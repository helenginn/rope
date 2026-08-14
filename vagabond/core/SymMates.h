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

#ifndef __vagabond__SymMates__
#define __vagabond__SymMates__

class AtomGroup;

namespace SymMates
{
	struct Mates
	{
		// within live_distance of the reference group - candidates for
		// full, independent coordination/H-bond treatment.
		AtomGroup *live = nullptr;
		// beyond live_distance but within dead_distance - covalent
		// topology only (never coordinated/searched), so atoms just
		// inside live_distance have their own real covalent bond
		// partners available via Atom::symmetryEquivalent().
		AtomGroup *dead = nullptr;
	};

	// dead_distance must be >= live_distance.
	Mates getSymmetryMates(AtomGroup *const &other,
	                       const std::string &spg_name,
	                       const std::array<double, 6> &uc,
	                       float live_distance, float dead_distance);

	// convenience wrapper for callers with no dead-bucket concept of
	// their own - equivalent to getSymmetryMates(..., distance, distance).live.
	AtomGroup *getSymmetryMates(AtomGroup *const &other,
	                            const std::string &spg_name,
	                            const std::array<double, 6> &uc, float distance);
};

#endif
