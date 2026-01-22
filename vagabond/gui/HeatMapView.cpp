#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/maths.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>

HeatMapView::HeatMapView(Scene *prev, const struct EntropyForHeatMap &entropy) : Scene(prev), _entropy(entropy)
{

}

void HeatMapView::setup()
{
    if (_entropy.dataMatrix.size() == 0)
    { 
//        return;
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
        }
    }

    _pcaMatrix = PCA::Matrix(_entropy.dataMatrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    addObject(_plot);
}
