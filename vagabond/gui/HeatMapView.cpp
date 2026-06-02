#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/maths.h>
#include <vagabond/utils/DoJob.h>
#include <HeatMapView.h>
#include <MatrixPlot.h>
#include <MatrixBox.h>
#include <ColourLegend.h>

HeatMapView::HeatMapView(Scene *prev, const std::vector<PathGroup> &paths, struct FlagParameters flagPar) : Scene(prev)
{
    _entropy = new Entropy(paths, flagPar);
    _flagPar = flagPar;

    _entropyData = new Entropy::EntropyForHeatMap;
    //PCA::setupMatrix(&_pcaMatrix, _entropy->rows(), _entropy->cols());
}

HeatMapView::~HeatMapView()
{
}

void HeatMapView::setup()
{
    addTitle("Heat Map");    

    std::cout << "Populating data matrix..." << std::endl;
    _entropy->populateHeatMap(_entropyData);

    if (_flagPar.timeDivisions > 1)
    {   
        std::cout << "adding slider..." << std::endl;
        setupSlider(_flagPar.timeDivisions);
    }

    {
        TextButton *t = new TextButton("Sum entropy", this);
        t->setRight(0.9, 0.1);
        t->setReturnTag("sum");
        addObject(t);
    }  
	
	redrawHeatMap(+0.1f);
}

void HeatMapView::redrawHeatMap(double num)
{
    if(_plot)
    {
        removeObject(_plot);
    }

    int t = (int) num;

    int rows = _entropy->rows();
    int cols = _entropy->cols();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Constant(rows, cols, NAN);
    
    std::vector<double> entropyVals(_entropyData->total[t].size());

    double maxEntVal = _entropyData->total[t].front();
    double minEntVal = _entropyData->total[t].front();

    for(int i = 0; i < _entropyData->total[t].size(); i++)
    {
        if(_entropyData->total[t][i] > maxEntVal)
        {
            maxEntVal = _entropyData->total[t][i];
        }
        if(_entropyData->total[t][i] < minEntVal)
        {
            minEntVal = _entropyData->total[t][i];
        }
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if(!(isnan(_entropyData->dataMatrix[t](i,j))))
            {
			    matrix(i, j) = maxEntVal - _entropyData->dataMatrix[t](i, j);
                matrix(i, j) = matrix(i, j)/(maxEntVal - minEntVal);
            }
        }
    }

    _pcaMatrix = PCA::Matrix(matrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    _plot->legend()->setScheme(Heat);

    MatrixBox* matBox = new MatrixBox(_plot, _entropy->rowNames(_entropyData->start), _entropy->colNames(_entropyData->end));

    addObject(_plot);
}

void HeatMapView::sumHeatMap()
{
    if(_plot)
    {
        removeObject(_plot);
    }

    if(_rangeSlider)
    {
        removeObject(_rangeSlider);
    }

    int rows = _entropy->rows();
    int cols = _entropy->cols();

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(rows, cols);
    
    std::vector<double> entropyVals(_entropyData->total[0].size());

    for (int t = 0; t < _entropyData->numDivisions; t++)
    {
        for(int i = 0; i < _entropyData->total[t].size(); i++) 
        {
            entropyVals[i] += _entropyData->total[t][i];
            
            std::cout << i << ": " << entropyVals[i] << std::endl;
        }
        matrix += _entropyData->dataMatrix[t];
    }
   
    double meanEntropy = mean(entropyVals);
    double stdEntropy = standard_deviation(entropyVals);
 
    double maxEntVal = entropyVals[0];
    double minEntVal = entropyVals[0];

    for(int i = 0; i < entropyVals.size(); i++)
    {
        if(entropyVals[i] > maxEntVal)
        {
            maxEntVal = entropyVals[i];
        }
        if(entropyVals[i] < minEntVal)
        {
            minEntVal = entropyVals[i];
        }
    }

    matrix = Eigen::MatrixXf::Ones(rows, cols) * maxEntVal - matrix;
    matrix = matrix/(maxEntVal - minEntVal);

    _pcaMatrix = PCA::Matrix(matrix);
	printMatrix(&_pcaMatrix);

    _plot = new MatrixPlot(_pcaMatrix, _mutex);
    _plot->setColourScheme(Heat);
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

void HeatMapView::buttonPressed(std::string tag, Button *button)
{
    if (tag == "sum")
    {
        sumHeatMap(); 
    }

    Scene::buttonPressed(tag, button);
}

void HeatMapView::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
 	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;

	if ((v.x < 0 || v.x > 1) || (v.y < 0 || v.y > 1))
	{
		Scene::mousePressEvent(x, y, button);
		return;
	}

	int left = v.x * _pcaMatrix.cols;
	int right = v.y * _pcaMatrix.rows;
	std::cout << left << " " << right << std::endl;
	setInformation(_entropyData->start[left]->model_id() + " to " + _entropyData->end[right]->model_id());
	Scene::mousePressEvent(x, y, button);
}

