#ifndef __vagabond__HeatMapView__
#define __vagabond__HeatMapView__

#include <mutex>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/paths/Entropy.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/gui/elements/DragResponder.h>

class MatrixPlot;
class Slider;

class HeatMapView : public Scene, public DragResponder
{
public:
    HeatMapView(Scene *prev, const std::vector<PathGroup> &paths, struct FlagParameters flagPar);
    ~HeatMapView();

    virtual void setup();
    
    void setupSlider(int timeDivisions);

    void redrawHeatMap(double num);
    void sumHeatMap();

    virtual void finishedDragging(std::string tag, double x, double y);
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
private:
    struct FlagParameters _flagPar{};

    Entropy *_entropy{};
    std::thread *_worker = nullptr;

    PCA::Matrix _pcaMatrix{};    
    MatrixPlot *_plot{};
    Slider *_rangeSlider;
    std::mutex _mutex;
};

#endif
