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

#include "ResultsPCAView.h"
#include <vagabond/gui/ColourScheme.h>
#include <algorithm>

ResultsPCAView::ResultsPCAView()
{
	setName("ResultsPCAView");
}

std::vector<glm::vec3> ResultsPCAView::pcaPoints(const Eigen::MatrixXf &matrix,
                                                 float extent)
{
	int n = matrix.rows();
	std::vector<glm::vec3> points(n, glm::vec3(0.f));

	if (n < 2 || matrix.cols() < 1)
	{
		return points;
	}

	Eigen::MatrixXf centred = matrix.rowwise() - matrix.colwise().mean();
	Eigen::BDCSVD<Eigen::MatrixXf> svd(centred, Eigen::ComputeThinU);
	Eigen::MatrixXf u = svd.matrixU();
	Eigen::VectorXf s = svd.singularValues();

	float furthest = 0;
	for (int i = 0; i < n; i++)
	{
		for (int a = 0; a < 3 && a < s.size(); a++)
		{
			points[i][a] = u(i, a) * s(a);
		}

		furthest = std::max(furthest, glm::length(points[i]));
	}

	if (furthest > 1e-6)
	{
		for (glm::vec3 &p : points)
		{
			p *= extent / furthest;
		}
	}

	return points;
}

std::vector<glm::vec4>
ResultsPCAView::scoreColours(const std::vector<float> &scores)
{
	std::vector<glm::vec4> colours;

	if (scores.empty())
	{
		return colours;
	}

	// by percentile rank of score (lowest is best): yellow at the 0th
	// percentile, fading to black at the median and staying black beyond
	size_t n = scores.size();
	std::vector<size_t> order(n);
	for (size_t i = 0; i < n; i++)
	{
		order[i] = i;
	}

	std::stable_sort(order.begin(), order.end(), [&scores](size_t a, size_t b)
	                 { return scores[a] < scores[b]; });

	std::vector<float> percentile(n, 0.f);
	for (size_t rank = 0; rank < n; rank++)
	{
		percentile[order[rank]] = (n > 1 ? (float)rank / (float)(n - 1) : 0.f);
	}

	ColourScheme *heat = new ColourScheme(BlackYellow);

	for (size_t i = 0; i < n; i++)
	{
		float goodness = std::max(0.f, 1.f - percentile[i] / 0.5f);
		colours.push_back(heat->colour(goodness));
	}

	delete heat;
	return colours;
}

void ResultsPCAView::setPoints(const std::vector<glm::vec3> &points,
                               const std::vector<glm::vec4> &colours)
{
	clearVertices();

	// drawn last-to-first: the results arrive best first, so the brightest
	// (best) points end up drawn last and on top. Vertex v is result n-1-v.
	for (size_t k = 0; k < points.size(); k++)
	{
		size_t i = points.size() - 1 - k;
		// point type 1 is the filled star, 0 the filled circle
		addPoint(points[i], _starred.count(i) ? 1 : 0);

		if (i < colours.size())
		{
			_vertices.back().color = colours[i];
		}
	}

	// the numbers the mouse picks points out by
	reindex();
	rebufferVertexData();
	forceRender();
}

bool ResultsPCAView::rowOf(int idx, size_t &row)
{
	if (idx < 0 || idx >= (int)vertexCount())
	{
		return false;
	}

	row = vertexCount() - 1 - idx;
	return true;
}

void ResultsPCAView::selected(int idx, bool inverse)
{
	size_t row;
	if (_select && rowOf(idx, row))
	{
		_select(row, inverse);
	}
}

void ResultsPCAView::interacted(int idx, bool hover, bool left)
{
	size_t row;
	if (!hover && left && _click && rowOf(idx, row))
	{
		_click(row);
	}
}

void ResultsPCAView::setStarred(const std::vector<size_t> &starred)
{
	_starred = std::set<size_t>(starred.begin(), starred.end());

	size_t n = vertexCount();
	for (size_t v = 0; v < n; v++)
	{
		setPointType(v, _starred.count(n - 1 - v) ? 1 : 0);
	}

	rebufferVertexData();
	forceRender();
}

void ResultsPCAView::extraUniforms()
{
	PointyView::extraUniforms();

	glm::mat4x4 shifted = glm::translate(glm::mat4(1.f),
	                                     glm::vec3(_shift, 0.f, 0.f)) * _proj;
	GLuint u = glGetUniformLocation(_program, "projection");
	glUniformMatrix4fv(u, 1, GL_FALSE, &shifted[0][0]);
}
