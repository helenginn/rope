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

#include <set>
#include <vagabond/utils/FileReader.h>
#include "PathMatrixView.h"
#include "MatrixBox.h"
#include "MatrixPlot.h"
#include "PathManager.h"
#include "Instance.h"
#include "Path.h"

// kJ/mol, as PathManager::obstacles() uses to weight energies
static const float kT = 2.57;

PathMatrixView::PathMatrixView(Scene *prev, const std::vector<PathGroup> &groups)
: Scene(prev)
{
	prepareMatrix(groups);
}

void PathMatrixView::prepareMatrix(const std::vector<PathGroup> &groups)
{
	// each selected group is a set of paths between one pair of instances
	std::set<std::pair<Instance *, Instance *>> unique;
	std::vector<std::pair<Instance *, Instance *>> pairs;

	for (const PathGroup &group : groups)
	{
		for (Path *const &path : group)
		{
			Instance *first = path->startInstance();
			Instance *second = path->endInstance();

			if (!first || !second || unique.count({first, second}))
			{
				continue;
			}

			unique.insert({first, second});
			pairs.push_back({first, second});
		}
	}

	PathManager::EnergyMatrix results;
	results = PathManager::manager()->pathMatrixFor(pairs);

	_matrix = PathManager::boltzmannMatrix(results, _names, kT, &_min, &_max);
}

void PathMatrixView::setup()
{
	addTitle("Van der Waals energies between paths");

	if (_names.size() == 0)
	{
		setInformation("No paths with results in this selection");
		return;
	}

	if (!(_matrix.array() == _matrix.array()).any())
	{
		setInformation("No energies have been calculated for these paths");
		return;
	}

	_plot = new MatrixPlot(_matrix);
	_box = new MatrixBox(_plot, _names, _names, false, this);
	_box->setCentre(0.5, 0.6);
	_box->resize(1.2);
	addObject(_box);

	setInformation("Relative probability (Boltzmann, kT = " + f_to_str(kT, 2) +
	               ") of the mean vdw energy of paths from row to column,\n"
	               "relative to the row's path to itself: 1 is the lowest "
	               "energy (" + f_to_str(_min, 2) + "), highest energy is " +
	               f_to_str(_max, 2) + ". Grey: no paths.");
}
