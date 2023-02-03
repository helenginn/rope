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
#include "Residue.h"
#include "TorsionRef.h"

class Polymer;

/** \class RefinementInfo
 *  \brief contains information about how to replicate refinement setup */

namespace Refine
{
	struct RTA
	{
		ResidueId id;
		TorsionRef torsion;
		float angle;
		
		RTA(const ResidueTorsion &rt)
		{
			id = rt.residue->id();
			torsion = rt.torsion;
			float angle = 0;
		}
		
		static std::vector<RTA> vector_from_rts(std::vector<ResidueTorsion> &rts)
		{
			std::vector<RTA> ret;
			ret.reserve(rts.size());
			for (const ResidueTorsion &rt : rts)
			{
				ret.push_back(rt);
			}
			
			return ret;
		}
		
		static std::vector<Angular> angulars_from_rts(std::vector<RTA> &rtas)
		{
			std::vector<Angular> angles;

			for (const Refine::RTA &rta : rtas)
			{
				angles.push_back(rta.angle);
			}
			
			return angles;
		}
	};

	struct Axis
	{
		std::vector<RTA> angles;
	};

	struct Info
	{
		std::string mol_id;
		Polymer *molecule = nullptr;
		std::vector<Axis> axes;
		int samples = 120;
		
		std::vector<float> mean;
	};
};

#endif
