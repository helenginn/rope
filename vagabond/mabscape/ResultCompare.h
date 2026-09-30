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

#ifndef __vagabond__ResultCompare__
#define __vagabond__ResultCompare__

#include "PosToComp.h"

/** how similar two refinement Results are. For each non-fixed antibody, its
 *  position in the left result is compared with each of its symmetry-related
 *  copies in the right result, keeping the closest (i.e. every antibody is
 *  free to match under its own symmetry operation). That distance is scored
 *  as in PosToComp (1 when on top of each other, decaying with distance;
 *  radius sets how quickly) and the scores are averaged over the antibodies.
 *  Fixed antibodies are left out, being identical in every result. */
float compareResults(const PosToComp::Result &left,
                     const PosToComp::Result &right,
                     float radius = 11.f);

#endif
