#include "HeatMapVDW.h"
#include "MatrixPlot.h"
#include "MatrixBox.h"
#include "PathGroup.h"
#include <vagabond/utils/maths.h>
#include <vagabond/gui/elements/TextButton.h>

HeatMapVDW::HeatMapVDW(Scene *prev, const std::vector<PathGroup> &paths) : Scene(prev)
{
    int rows, cols;
    std::map<Instance*, std::size_t> rowMap, colMap;

    std::vector<PathGroup> sortedPaths = PathGroup::alphabetise(paths);
    PathGroup::PathMatrixDims dims = PathGroup::matricise(sortedPaths);

    for (const auto& [rowInst, colInst] : dims)
    {
        rowMap.try_emplace(rowInst, rowMap.size());
        colMap.try_emplace(colInst, colMap.size());
    }

    int n = std::max(rowMap.size(), colMap.size());

	_squareData = Eigen::MatrixXf(n, n);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			// distinguishable per-cell values so a dragged row/column's
			// values can be visually traced to where they end up.
			_squareData(i, j) = i * 0.1 + j * 0.1;
		}
	}

    for (const auto& [inst, index] : rowMap)
    {
        _rowNames.push_back(inst->desc());
    }

    for (const auto& [inst, index] : colMap)
    {
        _colNames.push_back(inst->desc());
    }

	_rectData = Eigen::MatrixXf::Constant((int)_rowNames.size(), (int)_colNames.size(), NAN);
    
    std::vector<float> rectDataVector;

	for (PathGroup& p : sortedPaths)
	{
        auto r = rowMap[p.front()->startInstance()];
        auto c = colMap[p.front()->endInstance()];
		_rectData(r, c) = p.averageMetrics();

        rectDataVector.push_back(p.averageMetrics());
	}

    scaleMatrix(rectDataVector);
}

void HeatMapVDW::scaleMatrix(std::vector<float> rectDataVector)
{
   float dataMean = mean(rectDataVector);
   float dataStd = standard_deviation(rectDataVector);

   for(Eigen::Index i = 0; i < _rectData.size(); i++)
   {
       if(!isnan(_rectData(i)))
       {
           _rectData(i) -= dataMean;
           _rectData(i) /= dataStd;
       }
   }
}

void HeatMapVDW::resetMatrix()
{
	// original index (0, 1) - "Alpha" row, "Beta" column - regardless of
	// how the display has been dragged around since construction.
	_squareBox->setOriginalMatrix(_rectData);
}

void HeatMapVDW::printMatrixToTerminal()
{
    std::cout << _rectData << std::endl;
}

void HeatMapVDW::setup()
{
	addTitle("van der Waals");

	_squarePlot = new MatrixPlot(_squareData);
	_squareBox = new MatrixBox(_squarePlot, _squareNames, _squareNames,
	                          false);
	_squareBox->setCentre(0.25, 0.5);
	addObject(_squareBox);

	_rectPlot = new MatrixPlot(_rectData);
	_rectBox = new MatrixBox(_rectPlot, _rowNames, _colNames, false);
	_rectBox->setCentre(0.75, 0.5);
	addObject(_rectBox);

	TextButton *setValue = new TextButton("restore original view", this);
	setValue->setLeft(0.1, 0.8);
	setValue->setReturnJob([this]() { resetMatrix(); });
	addObject(setValue);

    TextButton *printMatrix = new TextButton("print matrix to terminal", this);
    printMatrix->setLeft(0.1, 0.9);
    printMatrix->setReturnJob([this]() { printMatrixToTerminal(); });
    addObject(printMatrix);
}
