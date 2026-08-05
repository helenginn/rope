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
#include <vagabond/gui/elements/GLView.h>

ClusterPlot::ClusterPlot(const Eigen::MatrixXf &dist,
                         const std::vector<float> &sizeWeights,
                         const glm::mat4x4 &model,
                         const std::vector<bool> &starred,
                         const std::vector<glm::vec3> &colours)
{
	setName("ClusterPlot");
	setSelectable(true);

	// PointyView's own constructor (just run) defaults to projected
	// rendering, meant for point clouds viewed through a real 3D camera
	// (e.g. ClusterView, ConfSpaceView's SVD cluster space) - this plot
	// is a flat, screen-fraction-positioned element like Graph's own
	// Scatter (same point.vsh/point.fsh pair, same convention), so that
	// default needs overriding here.
	setUsesProjection(false);

	// PointyView's default _size (40, further scaled by Window::ratio()/2)
	// is tuned for ClusterView's real-camera 3D cluster space, a
	// completely different scale from this flat plot's own tightly-bounded
	// screen-fraction area - left uncustomized, the base size (before any
	// per-node normal.x scaling even applies) likely exceeds the GPU
	// driver's max supported point size and clamps, which would make
	// per-node size variation invisible even though gl_PointSize is
	// genuinely being computed differently per vertex. Deliberately much
	// smaller; a first guess, likely needs visual tuning.
	_size = 12.f;

	_dist = dist;

	_shifter = new PositionShifter(model);

	// see _displayScale's comment (header) - the physics runs in "sim
	// space" where 1 sim unit is a sensible minimum separation, and this
	// picks the conversion factor so THIS matrix's widest pairwise
	// distance maps to about three-quarters of a screen unit.
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
	// dominate the gradient and flatten out local structure. Weighting
	// each pair inversely by the square of its own target (standard
	// Kamada-Kawai-style graph layout) makes close pairs dominate the
	// gradient and far pairs barely constrain it, instead of every pair
	// pulling equally hard.
	_shifter->setWeightFn([this](void *left, void *right) -> float
	{
		float target = targetFor(left, right);
		return 1.f / (target * target);
	});

	int n = (int)_dist.rows();

	// distinct starting positions only - PositionShifter's spring
	// contribution divides by the current separation between two
	// elements, so coincident starting points would be a divide-by-zero.
	// Seeded in sim-space at the same scale as the targets (see
	// _displayScale) rather than an arbitrarily small screen-space
	// circle, so the physics doesn't have to close a huge sim-space gap
	// in its first few steps - that gap previously blew up into NaN/Inf
	// momentum and froze every node in place for good.
	float radius = 0.5f / _displayScale;

	// sizeWeights isn't assumed normalised to any fixed range - normalise
	// against the largest entry here so the size scale is stable
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

	// point.vsh's own per-vertex size scale (normal.x, see its comment) -
	// 0 leaves a node at the shared uniform size, so this range sits a
	// modest, roughly symmetric spread either side of that baseline.
	const float minScale = -0.4f;
	const float maxScale = 0.6f;

	_baseColours.resize(n, glm::vec4(0.2f, 0.2f, 0.2f, 1.f));
	_selected.resize(n, false);

	for (int i = 0; i < n; i++)
	{
		bool star = (i < (int)starred.size() && starred[i]);

		float angle = 6.283185f * (float)i / (float)n;
		glm::vec3 simStart = glm::vec3(radius * cosf(angle),
		                               radius * sinf(angle), 0.f);
		glm::vec3 start = simStart * _displayScale;

		// points.png index 0 = filled circle, 1 = filled star - see
		// Graph::addPoint's own comment on the full icon set.
		addPoint(start, star ? 1 : 0);

		Snow::Vertex &vert = _vertices.back();

		float weight = (maxWeight > 0.f && i < (int)sizeWeights.size())
		? sizeWeights[i] / maxWeight : 1.f;
		vert.normal.x = minScale + (maxScale - minScale) * weight;

		if (i < (int)colours.size())
		{
			const glm::vec3 &c = colours[i];
			vert.color = glm::vec4(c, 1.f);
		}

		_baseColours[i] = vert.color;

		// PositionShifter only ever uses this as an opaque map key
		// (never dereferenced), so an encoded index avoids needing a
		// separate stable-address allocation per node. +1 so no node
		// ever gets the nullptr key.
		void *key = (void *)(intptr_t)(i + 1);
		_nodeIndex[key] = i;

		auto get_pos = [key, this]() -> glm::vec3
		{
			int idx = _nodeIndex.at(key);
			return _vertices[idx].pos / _displayScale;
		};
		auto set_pos = [key, this](const glm::vec3 &pos)
		{
			int idx = _nodeIndex.at(key);
			_vertices[idx].pos = pos * _displayScale;
		};

		_shifter->addPosition(key, get_pos, get_pos, set_pos);
	}

	// without this, the screen only actually redraws on unrelated input
	// (e.g. mouse movement) - _gl is null until this object's first
	// render() call sets it (Renderable::render()), so this Tidy job is a
	// no-op for whatever few cycles land before that. forceRender() only
	// sets dirty flags (safe from this background physics thread); the
	// actual GPU reupload happens on the next real render pass.
	_shifter->addTidy([this]()
	{
		forceRender(true, false);

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
	// stops and joins the physics thread before this object's own
	// destruction - its Tidy job and getters/setters still hold a raw
	// this pointer into _vertices/_nodeIndex.
	delete _shifter;
	_shifter = nullptr;
}

void ClusterPlot::updatePoints()
{
	for (const auto &pair : _nodeIndex)
	{
		int idx = pair.second;
		_vertices[idx].pos = _shifter->getPosition(pair.first) * _displayScale;
	}
}

void ClusterPlot::selected(int idx, bool inverse)
{
	if (idx < 0 || idx >= (int)_vertices.size())
	{
		return;
	}

	_selected[idx] = !inverse;

	// same highlight convention ClusterView::applySelected() uses.
	_vertices[idx].color = _selected[idx] ?
	glm::vec4(1.0f, 1.0f, 0.1f, 1.0f) : _baseColours[idx];

	forceRender(true, false);
}

std::vector<int> ClusterPlot::selectedIndices() const
{
	std::vector<int> result;

	for (size_t i = 0; i < _selected.size(); i++)
	{
		if (_selected[i])
		{
			result.push_back((int)i);
		}
	}

	return result;
}
