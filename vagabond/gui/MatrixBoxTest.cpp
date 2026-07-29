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

#include "MatrixBoxTest.h"
#include "MatrixPlot.h"
#include "MatrixBox.h"
#include <vagabond/gui/elements/TextButton.h>

MatrixBoxTest::MatrixBoxTest(Scene *prev) : Scene(prev)
{
	_squareNames = {"Alpha", "Beta", "Gamma", "Delta"};
	int n = (int)_squareNames.size();
	PCA::setupMatrix(&_squareData, n, n);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			// distinguishable per-cell values so a dragged row/column's
			// values can be visually traced to where they end up.
			_squareData[i][j] = i * 0.1 + j * 0.1;
		}
	}

	_rowNames = {"Row0", "Row1", "Row2"};
	_colNames = {"Col0", "Col1", "Col2", "Col3", "Col4"};
	PCA::setupMatrix(&_rectData, (int)_rowNames.size(),
	                 (int)_colNames.size());

	for (size_t i = 0; i < _rowNames.size(); i++)
	{
		for (size_t j = 0; j < _colNames.size(); j++)
		{
			_rectData[i][j] = i * 0.1 + j * 0.1;
		}
	}
}

void MatrixBoxTest::setValueAtOriginal()
{
	// original index (0, 1) - "Alpha" row, "Beta" column - regardless of
	// how the display has been dragged around since construction.
	_squareBox->setValueAtOriginal(0, 1, 99);
}

void MatrixBoxTest::setup()
{
	addTitle("Matrix box test");

	_squarePlot = new MatrixPlot(_squareData);
	_squareBox = new MatrixBox(_squarePlot, _squareNames, _squareNames,
	                          false);
	_squareBox->setCentre(0.25, 0.5);
	addObject(_squareBox);

	_rectPlot = new MatrixPlot(_rectData);
	_rectBox = new MatrixBox(_rectPlot, _rowNames, _colNames, false);
	_rectBox->setCentre(0.75, 0.5);
	addObject(_rectBox);

	TextButton *setValue = new TextButton("set Alpha/Beta to 99", this);
	setValue->setLeft(0.1, 0.8);
	setValue->setReturnJob([this]() { setValueAtOriginal(); });
	addObject(setValue);
}
