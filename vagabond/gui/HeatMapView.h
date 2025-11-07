#ifndef __vagabond__HeatMapView__
#define __vagabond__HeatMapView__

#include <mutex>
#include <vagabond/core/Entropy.h>
#include <vagabond/utils/Eigen/Dense>

class MatrixPlot;

class HeatMapView : public Scene
{
public:
    HeatMapView(Scene *prev, struct Entropy &entropy);

    virtual void setup();

private:
    struct Entropy _entropy;

    PCA::Matrix _pcaMatrix{};
    
    MatrixPlot *_plot;
    std::mutex _mutex;
};

#endif
