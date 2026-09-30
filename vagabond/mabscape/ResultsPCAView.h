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

#ifndef __vagabond__ResultsPCAView__
#define __vagabond__ResultsPCAView__

#include <vagabond/gui/PointyView.h>
#include <vagabond/utils/Eigen/Dense>
#include <set>
#include <vector>

/** One point per row of a (symmetric) matrix, placed at the first three
 *  principal components of that matrix, in a real 3D camera so that the
 *  hosting scene (a Mouse3D) can rotate it. Deliberately knows nothing about
 *  what the matrix represents. */
class ResultsPCAView : public PointyView
{
public:
	ResultsPCAView();

	// first three principal components of each row of the matrix, centred on
	// the origin and scaled so the furthest point is about `extent` away.
	// Any missing axis (fewer than three components) is left at zero.
	static std::vector<glm::vec3> pcaPoints(const Eigen::MatrixXf &matrix,
	                                        float extent = 1.5f);

	// the heat colour scheme, white for the best (lowest) score through to
	// black for the worst. Builds a ColourScheme, so main thread only.
	static std::vector<glm::vec4> scoreColours(const std::vector<float> &scores);

	// replaces all points; must be called from the main thread.
	void setPoints(const std::vector<glm::vec3> &points,
	               const std::vector<glm::vec4> &colours);

	// draws these points as stars rather than dots, and any others as dots.
	// Kept across setPoints() calls; must be called from the main thread.
	void setStarred(const std::vector<size_t> &starred);

	virtual void makePoints() {};
	virtual void updatePoints() {};
protected:
	// the camera rotates around the middle of the screen, so to keep the
	// shape turning in place while it sits to the right of the matrix, the
	// sideways offset is applied after the rotation, to the projection.
	virtual void extraUniforms();
private:
	// offset in normalised device coordinates (-1 to 1 across the screen)
	float _shift{0.5f};
	std::set<size_t> _starred;
};

#endif
