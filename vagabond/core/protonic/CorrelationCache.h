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

#ifndef __vagabond__CorrelationCache__
#define __vagabond__CorrelationCache__

#include <memory>
#include <tuple>
#include <vector>
#include <vagabond/utils/Eigen/Dense>

class Correlative;

/** The finished (assembled and Floyd-Warshall gap-filled) correlation matrix
 *  for a Clique's subdivisions, kept on the Clique itself (see
 *  Clique::correlationCache()) so it outlives the ViewCorrelations scene
 *  that computed it - that scene is destroyed every time the user goes back
 *  to the H-bond analysis control. Transient: never serialised.
 *
 *  `signature` records what the matrix was derived from (see
 *  ViewCorrelations::matrixSignature()); the cache is only reused while the
 *  subdivisions still produce an identical signature. */
struct CorrelationCache
{
	typedef std::vector<std::tuple<const void *, size_t, int, float>>
	Signature;

	Signature signature;
	std::shared_ptr<Correlative> correlative;

	// the finished, gap-filled matrix - what the MatrixPlot shows and what
	// CommunicationAnalysis is handed (one copy: the display copy holds the
	// same values once filling has finished, so it isn't stored twice, and
	// the Correlative's own working matrices are released - see
	// Correlative::releaseMatrices()).
	Eigen::MatrixXf result;
};

#endif
