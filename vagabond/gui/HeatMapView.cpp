#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>

HeatMapView::HeatMapView(Scene *prev, struct EntropyForHeatMap &entropy) : Scene(prev), _entropy(entropy)
{

}

void HeatMapView::setup()
{
    if (_entropy.dataMatrix.size() == 0)
    { 
        return;
    }

    int rows = _entropy.dataMatrix.rows();
    int cols = _entropy.dataMatrix.cols();
	std::cout << rows << " x " << cols << std::endl;

    /*Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);

    int nonZero = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if(i < j)
            {
                matrix(i, j) =  _entropy.perRes[nonZero];
                nonZero++;
            }
        }
    }

	std::cout << "Matrix:" << std::endl;
	std::cout << matrix << std::endl;*/

    _pcaMatrix = PCA::Matrix(_entropy.dataMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    addObject(_plot);  
	_plot->update();
}
