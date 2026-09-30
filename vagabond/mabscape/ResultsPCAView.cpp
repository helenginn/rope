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
	const glm::vec4 yellow(1.f, 1.f, 0.f, 1.f);
	const glm::vec4 blue(0.f, 0.f, 1.f, 1.f);

	std::vector<glm::vec4> colours(scores.size(), blue);

	if (scores.empty())
	{
		return colours;
	}

	float lowest = *std::min_element(scores.begin(), scores.end());
	float top = *std::max_element(scores.begin(), scores.end());
	float span = 0.5f * (lowest + top) - lowest;

	for (size_t i = 0; i < scores.size(); i++)
	{
		float t = (span > 1e-6 ? (scores[i] - lowest) / span : 0.f);
		t = std::min(1.f, std::max(0.f, t));
		colours[i] = blue + t * (yellow - blue);
	}

	return colours;
}

void ResultsPCAView::setPoints(const std::vector<glm::vec3> &points,
                               const std::vector<glm::vec4> &colours)
{
	clearVertices();

	for (size_t i = 0; i < points.size(); i++)
	{
		addPoint(points[i], 0);

		if (i < colours.size())
		{
			_vertices.back().color = colours[i];
		}
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
