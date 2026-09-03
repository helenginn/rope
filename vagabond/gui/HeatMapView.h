#ifndef __vagabond__HeatMapView__
#define __vagabond__HeatMapView__

#include <mutex>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/paths/Entropy.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/gui/elements/DragResponder.h>

class MatrixBox;
class MatrixPlot;
class ColourLegend;
class Slider;

class HeatMapView : public Scene, public DragResponder
{
public:
    HeatMapView(Scene *prev, const std::vector<PathGroup> &paths, struct FlagParameters flagPar);
    ~HeatMapView();

    virtual void setup();
    
    void setupSlider(int timeDivisions);

    void redrawHeatMap(double num);
    void showMatBox(Eigen::MatrixXf matrix);

    void scaleMatrix(Eigen::MatrixXf &matrix, std::vector<float> entropyVals);
    void sumHeatMap();
    void printMatrixToTerminal();

    virtual void finishedDragging(std::string tag, double x, double y);
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
private:
    struct FlagParameters _flagPar{};
    struct Entropy::EntropyForHeatMap *_entropyData{};   

    std::vector<std::string> _squareNames;
	std::vector<std::string> _rowNames;
	std::vector<std::string> _colNames;

	Eigen::MatrixXf _squareData;
	Eigen::MatrixXf _rectData;

	MatrixPlot *_squarePlot{};
	MatrixPlot *_rectPlot{};

	MatrixBox *_squareBox{};
	MatrixBox *_rectBox{};

    Entropy *_entropy{};
    std::thread *_worker = nullptr;

    PCA::Matrix _displayMatrix{};    
    MatrixPlot *_plot{};
    std::mutex _mutex;

    Slider *_rangeSlider;
    ColourLegend *_legend = nullptr;
};

#endif
