#ifndef __vagabond__HeatMapView__
#define __vagabond__HeatMapView__

#include <mutex>
#include <vagabond/core/Entropy.h>
#include <vagabond/utils/Eigen/Dense>

class MatrixPlot;

class HeatMapView : public Scene
{
public:
    HeatMapView(Scene *prev, const struct EntropyForHeatMap &entropy);

    virtual void setup();

private:
    struct EntropyForHeatMap _entropy;

    PCA::Matrix _pcaMatrix{};
    PCA::Matrix _weirdMatrix{};
    
    MatrixPlot *_plot;
    MatrixPlot *_weird;
    std::mutex _mutex;
    std::mutex _wMutex;
};

#endif
