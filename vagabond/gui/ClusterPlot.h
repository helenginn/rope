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

#ifndef __vagabond__ClusterPlot__
#define __vagabond__ClusterPlot__

#include <vagabond/gui/elements/Box.h>
#include <vagabond/utils/Eigen/Dense>
#include <vector>
#include <map>

class PositionShifter;
class Image;

/** Lays out one node per row/column of an arbitrary N x N distance matrix
 *  via PositionShifter, using that matrix as the spring target directly
 *  (PositionShifter::setTargetFn()) rather than Euclidean init-distance,
 *  since these distances have no underlying "real" coordinates to derive
 *  init from. Deliberately has no notion of what the matrix represents -
 *  originally written for CertainStates::distanceMatrix() (states within
 *  one subnetwork), it's equally usable for e.g. inter-subnetwork
 *  overlap distance, or any other precomputed dissimilarity matrix. */
class ClusterPlot : public Box
{
public:
	// sizeWeights: optional, one entry per row/col of dist, used to scale
	// each node's dot (see makeNodes()'s comment) - pass an empty vector
	// for uniform sizing.
	// starred: optional, one entry per row/col of dist - true draws that
	// node as assets/images/star.png instead of the usual dot.png (see
	// makeNodes()). Pass an empty vector for plain dots throughout; this
	// class has no opinion on what "starred" means to the caller (e.g.
	// ViewCorrelations::showSubnetworkClustering() uses it to flag
	// subnetworks touching a particular watched signal).
	// colours: optional, one entry per row/col of dist - tints that node
	// via Renderable::setColour() instead of leaving it at the image's own
	// default colour. Pass an empty vector (or leave any given index
	// unset) to leave that node's default colour alone; this class has no
	// opinion on what a colour means to the caller (e.g.
	// ViewCorrelations::showStateClustering() uses it to show what each
	// state assigns to a particular watched signal).
	ClusterPlot(const Eigen::MatrixXf &dist,
	           const std::vector<float> &sizeWeights,
	           const glm::mat4x4 &model,
	           const std::vector<bool> &starred = {},
	           const std::vector<glm::vec3> &colours = {});
	~ClusterPlot();

	// starts the physics thread - deliberately not done by the
	// constructor: callers position the whole plot afterwards (e.g.
	// setPosition()), which rigidly translates every already-constructed
	// child node via addToVertices() - if the physics thread were already
	// running at that point, that translation would race the thread's
	// own concurrent, unsynchronized vertex writes to those same nodes.
	// Note: position this plot via setPosition(), never setCentre()/
	// setArbitrary() - their recursive addAlign()/realign() cascade calls
	// setPosition() directly on every child too, using each child's own
	// (uninitialized) _x/_y fraction fields, which forces every single
	// node to the exact same screen position and destroys the physics-
	// driven layout entirely.
	void start();

private:
	void makeNodes(const std::vector<float> &sizeWeights,
	              const std::vector<bool> &starred,
	              const std::vector<glm::vec3> &colours);

	// spring target (sim-space units) for a pair of nodes, shared between
	// setTargetFn() and setWeightFn() - see PositionShifter.h.
	float targetFor(void *left, void *right) const;

	Eigen::MatrixXf _dist;

	// PositionShifter::gradient() floors any target below 1 up to 1 -
	// written for real atomic distances, always far above that. Our raw
	// distance counts have no inherent length unit, so rather than
	// fighting that floor, the simulation runs in its own "sim space"
	// (closest realistic pair = 1 sim unit apart, by construction always
	// above the floor) and this converts to/from screen space in the
	// getter/setter - chosen per-plot so the widest pairwise distance in
	// _dist maps to about half a screen unit, regardless of whether the
	// raw counts span 0-5 or 0-30.
	float _displayScale = 1.f;

	std::vector<Image *> _nodes;
	std::map<void *, int> _nodeIndex;

	PositionShifter *_shifter = nullptr;
};

#endif
