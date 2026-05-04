#ifndef __vagabond__HeatMapOptions__
#define __vagabond__HeatMapOptions__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/DragResponder.h>
#include <vagabond/core/PathEntropy.h>

class PathEntropy;
class PathGroup;
class Slider;

class HeatMapOptions : public Scene, public DragResponder
{
public:
    HeatMapOptions(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths);
    ~HeatMapOptions();

    virtual void setup();
    virtual void refresh();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual void finishedDragging(std::string tag, double x, double y);

private:
    void prepareProgress(int ticks, std::string text);
    void loadOptions();

    PathEntropy *_pathEntropy = nullptr;
    struct FlagParameters _flagPar;
    Entity *_entity = nullptr;

    Slider *_pathsSlider;
    Slider *_timeSlider;
};

#endif
