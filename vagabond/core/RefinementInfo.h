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

#ifndef __vagabond__RefinementInfo__
#define __vagabond__RefinementInfo__

#include <vagabond/c4x/Angular.h>
#include "ResidueTorsion.h"
#include "Residue.h"
#include "TorsionRef.h"
#include "RTAngles.h"

class Warp;

/** \class RefinementInfo
 *  \brief contains information about how to replicate refinement setup */

namespace Refine
{
	struct Info
	{
		std::string mol_id;
		Instance *instance = nullptr;
		int samples = 120;
		int master_dims = 3;
		Warp *warp = nullptr;
	};
};

#endif
