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

/** The other way to compare Results. Each antibody's positions in all the
 *  Results are divided into a cluster for each symmetry operator (see
 *  Positions::cluster()). In each Result, the vector from the centre of a
 *  cluster to where the antibody sits in it says which way, and how far, that
 *  Result pulls that symmetry-related antibody from the average. */
struct VectorModel
{
	// for each antibody, how sharp its clusters are (Positions::silhouette())
	std::vector<float> weights;
	// [antibody][result][cluster]: zero where a result has no position there
	std::vector<std::vector<std::vector<glm::vec3>>> vectors;
};

VectorModel buildVectorModel(const std::vector<PosToComp::Result> &results);

/** How alike two Results pull their antibodies: for each symmetry operator,
 *  the dot product of their vectors divided by the product of their lengths
 *  (the cosine of the angle between them, so -1 to 1 whatever the size),
 *  averaged over the operators, then over the antibodies weighted by the
 *  sharpness of each one's clusters. Fixed antibodies are left out, having no
 *  vectors. */
float compareByVectors(const VectorModel &model, size_t left, size_t right);

#endif
