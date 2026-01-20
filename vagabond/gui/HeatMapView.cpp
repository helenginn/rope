#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
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

    int nonZero = 0;
//"    _entropy.dataMatrix -= _entropy.dataMatrix.mean();
    double sigma = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (i != j)
            {
                std::cout << "NonZero!" << std::endl;
                sigma += pow(_entropy.dataMatrix(i,j), 2.0);
                nonZero++;
            }
        }
    }

    sigma = sqrt(sigma/nonZero);

//    _entropy.dataMatrix = ((_entropy.dataMatrix - mean * Eigen::MatrixXf::Identity(rows, cols))/sigma);
    _entropy.dataMatrix -= 0.5 * Eigen::MatrixXf::Identity(rows, cols);

    _pcaMatrix = PCA::Matrix(_entropy.dataMatrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    addObject(_plot);
}
