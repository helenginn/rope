#include "HeatMapVDW.h"
#include "MatrixPlot.h"
#include "MatrixBox.h"
#include <vagabond/gui/elements/TextButton.h>

HeatMapVDW::HeatMapVDW(Scene *prev, const std::vector<PathGroup> &paths) : Scene(prev)
{
/*	int n = (int)paths.size();
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
*/}

/*void MatrixBoxTest::setValueAtOriginal()
{
	// original index (0, 1) - "Alpha" row, "Beta" column - regardless of
	// how the display has been dragged around since construction.
	_squareBox->setValueAtOriginal(0, 1, 99);
}
*/
void HeatMapVDW::setup()
{
	addTitle("van der Waals");

/*	_squarePlot = new MatrixPlot(_squareData);
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
*/}
