#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/VagWindow.h>
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
}

HeatMapView::~HeatMapView()
{
}

void HeatMapView::setup()
{
    addTitle("Heat Map");    

    std::cout << "Setting up heat map..." << std::endl;
	auto gui_setup = [this]()
	{
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
	};

	auto do_heat_map = [this, gui_setup]()
	{
		std::cout << "Running do heat map..." << std::endl;

        VagWindow::window()->requestProgressBar(_entropy->ticks(), "Generating heatmap");

		_entropy->populateHeatMap(_entropyData);
		std::cout << "Now preparing to set up GUI elements" << std::endl;
		addMainThreadJob(gui_setup);
	};

	addMainThreadJob([this, do_heat_map]()
			{
				DoJob job(do_heat_map);
			});
}

void HeatMapView::redrawHeatMap(double num)
{
    deleteTemps();

    int t = (int) num;

    int rows = _entropy->rows();
    int cols = _entropy->cols();

    Eigen::MatrixXf matrix = _entropyData->dataMatrix[t];
    
    std::cout << "Matrix loaded from entropyData..." << std::endl;   
    _displayMatrix = PCA::Matrix(matrix);
	printMatrix(&_displayMatrix);

    std::vector<double> entropyVals(_entropyData->total.size());

    for (int i = 0; i < _entropyData->total.size(); i++)
    {
        entropyVals[i] += _entropyData->total[i][t];
    }
   
    scaleMatrix(matrix, entropyVals);
   
    std::cout << "Matrix after scaling..." << std::endl;   
    _displayMatrix = PCA::Matrix(matrix);
	printMatrix(&_displayMatrix);

    showMatBox(matrix);
}

void HeatMapView::sumHeatMap()
{
    deleteTemps();

    if(_rangeSlider)
    {
        removeObject(_rangeSlider);
    }

    Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(_entropy->rows(), _entropy->cols());
    
    for (int t = 0; t < _entropyData->numDivisions; t++)
    {
        matrix += _entropyData->dataMatrix[t];
    }

    for (int i = 0; i < _entropy->rows(); i++)
    {
        for (int j = 0; j < _entropy->cols(); j++)
        {
            if(!(isnan(_entropyData->dataMatrix[0](i,j))))
            {
                matrix(i, j) /= _entropyData->numDivisions;
            }
        }
    }

    std::vector<double> entropyVals(_entropyData->total.size());

    for (int i = 0; i < _entropyData->total.size(); i++)
    {
        for(int t = 0; t < _entropyData->numDivisions; t++) 
        {
            entropyVals[i] += _entropyData->total[i][t];
        }

        entropyVals[i] /= _entropyData->numDivisions;
    }
 
    scaleMatrix(matrix, entropyVals);
    showMatBox(matrix);
}

void HeatMapView::scaleMatrix(Eigen::MatrixXf &matrix, std::vector<double> entropyVals)
{
    double meanEntropy = mean(entropyVals);
    double stdEntropy = standard_deviation(entropyVals);
 
    for (int i = 0; i < _entropy->rows(); i++)
    {
        for (int j = 0; j < _entropy->cols(); j++)
        {
            if(!(isnan(_entropyData->dataMatrix[0](i,j))))
            {
			    matrix(i, j) = matrix(i, j) - meanEntropy;
                matrix(i, j) = matrix(i, j)/(stdEntropy);
            }
        }
    }
}

void HeatMapView::showMatBox(Eigen::MatrixXf matrix)
{
    _displayMatrix = PCA::Matrix(matrix);
	printMatrix(&_displayMatrix);

    _plot = new MatrixPlot(_displayMatrix);
    _plot->setColourScheme(ZScore);

    std::vector<std::string> rowNames = _entropy->startNames();
    std::vector<std::string> colNames = _entropy->endNames();

    std::reverse(rowNames.begin(), rowNames.end());

    MatrixBox *matBox = new MatrixBox(_plot, colNames, rowNames, true);
    matBox->setCentre(0.5, 0.5);
    addTempObject(matBox);
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

	int left = v.x * _displayMatrix.cols;
	int right = v.y * _displayMatrix.rows;
	std::cout << left << " " << right << std::endl;
    
    auto names = _entropy->names(left, right);

	setInformation(names.first + " vs " + names.second);
	Scene::mousePressEvent(x, y, button);
}

