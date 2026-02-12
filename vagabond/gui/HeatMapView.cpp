#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/maths.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>
#include <ColourLegend.h>

HeatMapView::HeatMapView(Scene *prev, const struct EntropyForHeatMap &entropy) : Scene(prev), _entropy(entropy)
{

}

void HeatMapView::setup()
{
    if (_entropy.timeDivisions > 1)
    { 
        setupSlider(_entropy.timeDivisions);
    }

    int rows = _entropy.dataMatrix.rows();
    int cols = _entropy.dataMatrix.cols();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);

    double meanEntropy = mean(_entropy.total);
    double stdEntropy = standard_deviation(_entropy.total);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            _entropy.dataMatrix(i,j)-=meanEntropy;
            _entropy.dataMatrix(i,j)/=stdEntropy;

            _entropy.dataMatrix(i,j)-=0.5;
        }
    }

    _pcaMatrix = PCA::Matrix(_entropy.dataMatrix);
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
    addObject(s);
}

void HeatMapView::finishedDragging(std::string tag, double x, double y)
{
}
