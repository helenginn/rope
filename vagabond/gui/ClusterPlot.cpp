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

#include "ClusterPlot.h"

#include <vagabond/core/PositionShifter.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/elements/GLView.h>

ClusterPlot::ClusterPlot(const Eigen::MatrixXf &dist,
                         const std::vector<float> &sizeWeights,
                         const glm::mat4x4 &model,
                         const std::vector<bool> &starred,
                         const std::vector<glm::vec3> &colours)
{
	_dist = dist;

	_shifter = new PositionShifter(model);

	// see _displayScale's comment (header) - the physics runs in "sim
	// space" where 1 sim unit is a sensible minimum separation, and this
	// picks the conversion factor so THIS matrix's widest pairwise
	// distance maps to about three-quarters of a screen unit (50% more
	// than the original half-unit target - there was room to spare
	// before the plot ran off screen, and the extra spread makes it
	// easier to distinguish close-together nodes).
	float maxRaw = _dist.size() ? _dist.maxCoeff() : 0.f;
	float maxTarget = 1.f + maxRaw;
	_displayScale = 0.75f / maxTarget;

	// spring targets come straight from _dist (in sim-space units) rather
	// than the default Euclidean-init-distance behaviour - see
	// PositionShifter::setTargetFn().
	_shifter->setTargetFn([this](void *left, void *right) -> float
	{
		return targetFor(left, right);
	});

	// a fully-connected graph of very unevenly-scaled targets (a couple
	// of tight local clusters plus many long-range pairs 10-20x larger)
	// otherwise lets the sheer number and magnitude of long-range pairs
	// dominate the gradient and flatten out local structure - reported as
	// "lots of dots lying on top of each other" even once the layout was
	// stably converging rather than frozen. Weighting each pair inversely
	// by the square of its own target (standard Kamada-Kawai-style graph
	// layout) makes close pairs dominate the gradient and far pairs
	// barely constrain it, instead of every pair pulling equally hard.
	_shifter->setWeightFn([this](void *left, void *right) -> float
	{
		float target = targetFor(left, right);
		return 1.f / (target * target);
	});

	makeNodes(sizeWeights, starred, colours);

	// without this, the screen only actually redraws on unrelated input
	// (e.g. mouse movement) - _gl is null until this object's first
	// render() call sets it (Renderable::render()), so this Tidy job is a
	// no-op for whatever few cycles land before that.
	_shifter->addTidy([this]()
	{
		if (_gl)
		{
			_gl->viewChanged();
		}
	});
}

void ClusterPlot::start()
{
	_shifter->run();
}

float ClusterPlot::targetFor(void *left, void *right) const
{
	if (!_nodeIndex.count(left) || !_nodeIndex.count(right))
	{
		return 1.f;
	}

	// +1 offset: gradient()'s own clamp (target < 1 -> 1) exists for real
	// atomic distances, always well above 1 - our raw distances often
	// start at (or near) 0, so left unmodified almost every pair would
	// get clamped to the *same* floor value, collapsing nearly all the
	// variation this plot exists to show.
	return 1.f + _dist(_nodeIndex.at(left), _nodeIndex.at(right));
}

ClusterPlot::~ClusterPlot()
{
	// stops and joins the physics thread before Box's own destructor gets
	// to delete the node Renderables its Tidy jobs and getters/setters
	// still hold raw pointers to.
	delete _shifter;
	_shifter = nullptr;
}

void ClusterPlot::makeNodes(const std::vector<float> &sizeWeights,
                            const std::vector<bool> &starred,
                            const std::vector<glm::vec3> &colours)
{
	int n = (int)_dist.rows();
	_nodes.resize(n);

	// distinct starting positions only - PositionShifter's spring
	// contribution divides by the current separation between two
	// elements, so coincident starting points would be a divide-by-zero.
	// Seeded in sim-space at the same scale as the targets (see
	// _displayScale) rather than an arbitrarily small screen-space
	// circle, so the physics doesn't have to close a huge sim-space gap
	// in its first few steps - that gap is exactly what previously blew
	// up into NaN/Inf momentum and froze every node in place for good.
	float radius = 0.5f / _displayScale;

	// sizeWeights isn't assumed normalised to any fixed range - normalise
	// against the largest entry here so the dot size scale is stable
	// regardless of the caller's own units (e.g. Boltzmann weights vs.
	// raw probe counts).
	float maxWeight = 0.f;
	for (float w : sizeWeights)
	{
		if (w > maxWeight)
		{
			maxWeight = w;
		}
	}

	const float minSize = 0.001f;
	const float maxSize = 0.025f;

	for (int i = 0; i < n; i++)
	{
		bool star = (i < (int)starred.size() && starred[i]);
		Image *dot = new Image(star ? "assets/images/star.png"
		                            : "assets/images/dot.png");

		float weight = (maxWeight > 0.f && i < (int)sizeWeights.size())
		? sizeWeights[i] / maxWeight : 1.f;
		dot->resize(minSize + (maxSize - minSize) * weight);

		if (i < (int)colours.size())
		{
			const glm::vec3 &c = colours[i];
			dot->setColour(c.x, c.y, c.z);
		}

		float angle = 6.283185f * (float)i / (float)n;
		glm::vec3 simStart = glm::vec3(radius * cosf(angle),
		                               radius * sinf(angle), 0.f);
		glm::vec3 start = simStart * _displayScale;
		dot->setPosition(start);

		addObject(dot);

		_nodes[i] = dot;
		_nodeIndex[dot] = i;

		// converts between screen space (what the Image actually renders
		// at) and sim space (what the physics/targetFn operate in) - see
		// _displayScale's comment.
		auto get_pos = [dot, this]() -> glm::vec3
		{
			return dot->centroid() / _displayScale;
		};
		auto set_pos = [dot, this](const glm::vec3 &pos)
		{
			dot->setPosition(pos * _displayScale);
		};

		_shifter->addPosition(dot, get_pos, get_pos, set_pos);
		_shifter->addTidy([dot]() { dot->forceRender(true, false); });
	}
}
