#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>

HeatMapView::HeatMapView(Scene *prev, struct Entropy &entropy) : Scene(prev), _entropy(entropy)
{

}

void HeatMapView::setup()
{
    if (_entropy.total.size() == 0)
    {
        return;
    }

    int rows = _entropy.start.size();
    int cols = _entropy.end.size();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);

    _pcaMatrix = PCA::Matrix(matrix);

    int nonZero = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if(i < j)
            {
                _pcaMatrix[i][j] =  _entropy.perRes[nonZero];
                nonZero++;
            }
        }
    }

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    addObject(_plot);  
}
