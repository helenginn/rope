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

#ifndef __vagabond__PathMatrixView__
#define __vagabond__PathMatrixView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/Eigen/Dense>
#include "PathGroup.h"

class MatrixBox;
class MatrixPlot;

/* The van der Waals results which path-matrix= writes to a file, as a
 * MatrixBox: for each selected pair of instances the mean energy of its
 * paths, relative to the same start instance's path to itself, shown as a
 * relative probability from the Boltzmann distribution (1 for the lowest
 * energy). Pairs without paths are left grey. */

class PathMatrixView : public Scene
{
public:
	PathMatrixView(Scene *prev, const std::vector<PathGroup> &groups);

	virtual void setup();
private:
	void prepareMatrix(const std::vector<PathGroup> &groups);

	std::vector<std::string> _names;

	// relative Boltzmann probabilities (0-1) for colouring, with NaN for
	// pairs without paths
	Eigen::MatrixXf _matrix;
	float _min = 0;
	float _max = 0;

	MatrixPlot *_plot{};
	MatrixBox *_box{};
};

#endif
