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

#ifndef __vagabond__PickBest__
#define __vagabond__PickBest__

#include <vagabond/utils/Eigen/Dense>
#include <set>
#include <vector>

/** Chooses a set of mutually similar, well-scoring results.
 *
 *  The set starts as the given fraction of results with the lowest scores
 *  (lower is better). It is then refined: the member which is most
 *  dissimilar from the rest of the set is swapped for the unmarked result
 *  which is most similar to that rest, for as long as the swap makes the
 *  set more similar overall. Since every swap strictly raises the total
 *  pairwise similarity, this always converges.
 *
 *  @param similarity square matrix, higher meaning more alike (the diagonal
 *  is ignored)
 *  @param scores one per row of the matrix, lower is better
 *  @param fraction share of all the results to choose (at least two are
 *  chosen, if there are that many)
 *  @param locked results which are always chosen, and never swapped out. If
 *  there are any, the rest are chosen for being most like these, instead of
 *  most like each other
 *  @return the indices of the chosen results, in ascending order */
std::vector<size_t> pickBest(const Eigen::MatrixXf &similarity,
                             const std::vector<float> &scores,
                             float fraction,
                             const std::set<size_t> &locked = {});

#endif
