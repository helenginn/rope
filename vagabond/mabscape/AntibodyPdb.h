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

#ifndef __vagabond__AntibodyPdb__
#define __vagabond__AntibodyPdb__

#include "CrossCheckView.h"
#include <string>
#include <vector>

/** Antibodies written out as a PDB file, one mercury atom for each, in the
 *  frame of the antigen they were mapped onto. Each symmetry copy (the
 *  alternative positions of an antibody) is a model of its own, holding every
 *  antibody at its position for that copy. The B factor is the RMSD of its
 *  positions in the results, as the cross-check works it out (0 for a fixed
 *  antibody, which does not move). The REMARK lines give every atom's
 *  antibody name, and the antigen PDB which is the reference frame. */
class AntibodyPdb
{
public:
	typedef CrossCheckView::Stats::Antibody Antibody;

	/** The chain and residue number of an antibody's atom: the chain is the
	 *  first letter of its name, and the residue number the digits in it (all
	 *  of them, run together). One which cannot be named so (no letter or no
	 *  digit, a number too big for a PDB file, or a chain and residue number
	 *  already taken by another antibody) gives up: it is put in chain Z, as
	 *  the next residue number free there, then in chain Y when Z is full, and
	 *  so on. */
	struct Naming
	{
		std::string chain;
		int residue{};
		bool gaveUp{};
	};

	static std::vector<Naming> assignNames(const std::vector<std::string> &names);

	// Throws if the file cannot be written
	static void write(const std::string &path, 
	                  const std::vector<Antibody> &antibodies,
	                  const std::string &reference, const std::string &title,
	                  size_t results);
};

#endif
