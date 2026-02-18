#include <vagabond/gui/HeatMapOptions.h>
#include <vagabond/core/Path.h>
#include <vagabond/core/PathEntropy.h>

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TickBoxes.h>

HeatMapOptions::HeatMapOptions(Scene *scene, const std::vector<Path*> paths) : Scene(scene)
{
    _paths = paths;

    _pathEntropy = new PathEntropy();
}

void HeatMapOptions::setup()
{
    addTitle("Select Calculation Parameters");

    struct FlagParameters flagPar = _pathEntropy->initFlagPar();

    {
        Slider *sPaths = new Slider();
        sPaths->setDragResponder(this);
        sPaths->setup("Number of paths", 2, 11, 1);
        addObject(sPaths);
    }

    {
        Slider *sTime = new Slider();
        sTime->setDragResponder(this);
        sTime->setup("Number of time divisions", 1, 10, 1);
        addObject(sTime);
    }

    {
        TickBoxes *tb = new TickBoxes(this, this);
        tb->addOption("mist");
        tb->arrange(0.4, 0.22, 0.9, 0.5);
        addObject(tb);
    }
}
