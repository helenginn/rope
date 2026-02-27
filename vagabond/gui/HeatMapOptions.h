#ifndef __vagabond__HeatMapOptions__
#define __vagabond__HeatMapOptions__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/DragResponder.h>
#include <vagabond/core/PathEntropy.h>

class Path;
class PathEntropy;

class Slider;

class HeatMapOptions : public Scene, public DragResponder
{
public:
    HeatMapOptions(Scene *prev, const std::vector<Path *> paths);
    ~HeatMapOptions();

    virtual void setup();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual void finishedDragging(std::string tag, int x);

private:
    PathEntropy *_pathEntropy = nullptr;
    struct FlagParameters _flagPar;
    std::vector<Path *> _paths;

    Slider *_pathsSlider;
    Slider *_timeSlider;
};

#endif
