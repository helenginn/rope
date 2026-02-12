#ifndef __vagabond__HeatMapView__
#define __vagabond__HeatMapView__

#include <mutex>
#include <vagabond/core/Entropy.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/gui/elements/DragResponder.h>

class MatrixPlot;
class Slider;

class HeatMapView : public Scene, public DragResponder
{
public:
    HeatMapView(Scene *prev, const struct EntropyForHeatMap &entropy);

    virtual void setup();
    void setupSlider(int timeDivisions);

    virtual void finishedDragging(std::string tag, double x, double y);

private:
    struct EntropyForHeatMap _entropy;

    PCA::Matrix _pcaMatrix{};
    
    MatrixPlot *_plot;
    Slider *_rangeSlider;
    std::mutex _mutex;
};

#endif
