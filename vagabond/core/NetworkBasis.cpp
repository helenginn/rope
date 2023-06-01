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

#include "NetworkBasis.h"
#include "SpecificNetwork.h"

NetworkBasis::NetworkBasis() : ConcertedBasis()
{

}

float NetworkBasis::contributionForAxis(BondSequence *seq,
                                        int tidx, int i, const float *vec)
{
	if (seq == nullptr)
	{
		return ConcertedBasis::contributionForAxis(seq, tidx, i, vec);
	}
	float t_all = _sn->torsionForIndex(seq, tidx, vec);

	// remove average, it'll be added back on
	float torsion = t_all - _angles[tidx].angle; 
	torsion /= 2;

	return torsion;
}
