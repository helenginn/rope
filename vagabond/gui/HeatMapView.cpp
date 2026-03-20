#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/maths.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>
#include <ColourLegend.h>

HeatMapView::HeatMapView(Scene *prev, const std::vector<struct EntropyForHeatMap> &entropy) : Scene(prev), _entropy(entropy)
{
    //_timeDivisions = _entropy.size();
}

void HeatMapView::setup()
{
    if (_entropy.size() > 1)
    { 
        setupSlider(_entropy.size());
    }

	redrawHeatMap(+0.1f);
/*
    int rows = _entropy[0].dataMatrix.rows();
    int cols = _entropy[0].dataMatrix.cols();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);

    double meanEntropy = mean(_entropy[0].total);
    double stdEntropy = standard_deviation(_entropy[0].total);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            _entropy[0].dataMatrix(i,j)-=meanEntropy;
            _entropy[0].dataMatrix(i,j)/=stdEntropy;

            _entropy[0].dataMatrix(i,j)-=0.5;
        }
    }

    _pcaMatrix = PCA::Matrix(_entropy[0].dataMatrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);

    _plot->legend()->setScheme(Heat);
    addObject(_plot);
*/
}

void HeatMapView::redrawHeatMap(double num)
{
    if(_plot)
    {
        removeObject(_plot);
    }

    int t = (int) num;

    int rows = _entropy[t].dataMatrix.rows();
    int cols = _entropy[t].dataMatrix.cols();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);

    double meanEntropy = mean(_entropy[t].total);
    double stdEntropy = standard_deviation(_entropy[t].total);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
			matrix(i, j) = _entropy[t].dataMatrix(i, j);
            matrix(i,j)-=meanEntropy;
            matrix(i,j)/=stdEntropy;

            matrix(i,j)-=0.5;
        }
    }

    _pcaMatrix = PCA::Matrix(matrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);

    _plot->legend()->setScheme(Heat);
    addObject(_plot);
}

void HeatMapView::setupSlider(int timeDivisions)
{
    Slider *s = new Slider();
    s->setDragResponder(this);
    s->resize(0.5);
    s->setup("Route point number", 1, timeDivisions, 1);
    s->setCentre(0.5, 0.85);
    _rangeSlider = s;
    addObject(s);
}

void HeatMapView::finishedDragging(std::string tag, double x, double y)
{
    double num = x - 1;
    
    redrawHeatMap(num);
}
