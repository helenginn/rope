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

#include "PointyView.h"
#include <vagabond/utils/Eigen/Dense>
#include <vector>
#include <map>

class PositionShifter;

/** Lays out one node per row/column of an arbitrary N x N distance matrix
 *  via PositionShifter, using that matrix as the spring target directly
 *  (PositionShifter::setTargetFn()) rather than Euclidean init-distance,
 *  since these distances have no underlying "real" coordinates to derive
 *  init from. Deliberately has no notion of what the matrix represents -
 *  originally written for CertainStates::distanceMatrix() (states within
 *  one subnetwork), it's equally usable for e.g. inter-subnetwork
 *  overlap distance, or any other precomputed dissimilarity matrix.
 *
 *  Renders as a single GL_POINTS buffer (PointyView) rather than one
 *  Image per node - the same one-Renderable-many-vertices approach
 *  Graph's own Scatter uses - so a plot with hundreds of nodes is one
 *  draw call and one small VBO instead of hundreds of separate quads.
 *  Selectable via the standard IndexResponder/SelectionBox mechanism
 *  (shift+drag or shift+click, see Mouse2D/IndexResponseView) once
 *  registered with the hosting Scene's addIndexResponder(). */
class ClusterPlot : public PointyView
{
public:
	// sizeWeights: optional, one entry per row/col of dist, used to scale
	// each node's point size (see makeNodes()'s comment) - pass an empty
	// vector for uniform sizing.
	// starred: optional, one entry per row/col of dist - true draws that
	// node with the filled-star icon (points.png index 1) instead of the
	// usual filled circle (index 0) - see makeNodes(). Pass an empty
	// vector for plain dots throughout; this class has no opinion on
	// what "starred" means to the caller (e.g.
	// ViewCorrelations::showSubnetworkClustering() uses it to flag
	// subnetworks touching a particular watched signal).
	// colours: optional, one entry per row/col of dist - tints that
	// node's base colour instead of the default grey. Pass an empty
	// vector (or leave any given index unset) to leave that node's
	// default colour alone; this class has no opinion on what a colour
	// means to the caller (e.g. ViewCorrelations::showStateClustering()
	// uses it to show what each state assigns to a particular watched
	// signal). Overridden visually (not replaced) while a node is
	// selected - see selected().
	ClusterPlot(const Eigen::MatrixXf &dist,
	           const std::vector<float> &sizeWeights,
	           const glm::mat4x4 &model,
	           const std::vector<bool> &starred = {},
	           const std::vector<glm::vec3> &colours = {});
	~ClusterPlot();

	// starts the physics thread - deliberately not done by the
	// constructor: callers position the whole plot afterwards (e.g.
	// setPosition()), which rigidly translates every already-constructed
	// node's vertex position via addToVertices() - if the physics thread
	// were already running at that point, that translation would race
	// the thread's own concurrent, unsynchronized vertex writes to those
	// same vertices.
	// Note: position this plot via setPosition(), never setCentre()/
	// setArbitrary() - see the same caution on the old Image-based
	// version this replaced, still applicable since PositionShifter's
	// setter writes vertex positions directly rather than going through
	// any per-child alignment fields.
	void start();

	virtual void makePoints() {};
	virtual void updatePoints();

	virtual bool selectable() const
	{
		return true;
	}

	/** IndexResponder callback - marks node idx selected (inverse false)
	 * or deselected (inverse true), visually highlighted the same way
	 * ClusterView marks a selected point. */
	virtual void selected(int idx, bool inverse);

	/** which node indices are currently selected - queried by the
	 * caller, since this class has no notion of what a node represents
	 * (see the class comment) and so cannot itself act on a selection,
	 * only track and display it. */
	std::vector<int> selectedIndices() const;

private:
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

	std::map<void *, int> _nodeIndex;
	std::vector<glm::vec4> _baseColours;
	std::vector<bool> _selected;

	PositionShifter *_shifter = nullptr;
};

#endif
