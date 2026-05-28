#include <vagabond/gui/HeatMapOptions.h>
#include <vagabond/gui/HeatMapView.h>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/paths/Entropy.h>

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>

#include "VagWindow.h"

HeatMapOptions::HeatMapOptions(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths) : Scene(prev)
{
    _entity = entity;
    _paths = paths;
 
    _pathEntropy = new PathEntropy();
    _flagPar = _pathEntropy->initFlagPar();
}

HeatMapOptions::~HeatMapOptions()
{
    delete _pathEntropy;
}

void HeatMapOptions::setup()
{
    addTitle("Select Calculation Parameters");

    {
        Text *t = new Text("Use MIST algorithm?");
        t->setLeft(0.15, 0.22);
        addObject(t);
    }
 
    {
        Text *t = new Text("Number of paths:");
        t->setLeft(0.15, 0.35);
        addObject(t);
    }
 
    {
        Text *t = new Text("Number of nearest neighbours:");
        t->setLeft(0.15, 0.5);
        addObject(t);
    }

    float bottom = 0.9;

    {
	    TextButton *t = new TextButton("Generate heat map", this);
	    t->setRight(0.9, bottom);
	    t->setReturnTag("heatmap");
	    addObject(t);
    }

    loadOptions();
}

void HeatMapOptions::loadOptions()
{
    deleteTemps();

    {
        TickBoxes *tb = new TickBoxes(this, this);
        tb->addOption("Yes", "mist");
        tb->arrange(0.8, 0.22, 0.9, 0.8);
        addTempObject(tb);
    }
   
    {
        Slider *sPaths = new Slider();
        sPaths->setDragResponder(this);
        sPaths->resize(0.5);
        sPaths->setup("Number of paths", 2, maxPaths(), 1);
        sPaths->setReturnTag("paths");
	    sPaths->setCentre(0.7, 0.35);
        addTempObject(sPaths);
    }

    {
        Slider *sNN = new Slider();
        sNN->setDragResponder(this);
        sNN->resize(0.5);
        sNN->setup("Number of nearest neighbours", 2, maxPaths()-1, 1);
        sNN->setReturnTag("neighbours");
	    sNN->setCentre(0.7, 0.5);
        addTempObject(sNN);
    }

    {
        Text *t = new Text("Number of timepoints:");
        t->setLeft(0.15, 0.65);
        addTempObject(t);
    }

    {
        Slider *sTime = new Slider();
        sTime->setDragResponder(this);
        sTime->resize(0.5);
        sTime->setup("Number of time divisions", 1, 10, 1);
        sTime->setReturnTag("timepoints");
	    sTime->setCentre(0.7, 0.65);
        addTempObject(sTime);
    }

}

void HeatMapOptions::buttonPressed(std::string tag, Button *button)
{
    if (tag == "mist")
    {
        TickBoxes *tb = static_cast<TickBoxes *>(button);

        if (tb->isTicked(tag))
        {
            _flagPar.mist = true;
        }
    }

    if (tag == "heatmap")
    {
		HeatMapView *view = new HeatMapView(this, _paths, _flagPar);
        view->show();
    }

    Scene::buttonPressed(tag, button);
}

void HeatMapOptions::finishedDragging(std::string tag, double x, double y)
{
    if(tag == "paths")
    {
        _flagPar.nf = lrint(x);
    }
    else if(tag == "neighbours")
    {
        _flagPar.n = lrint(x);
    }
    else if(tag == "timepoints")
    {
        _flagPar.timeDivisions = lrint(x);
    }
}

int HeatMapOptions::maxPaths()
{
    int max = _paths[0].size();

    for (PathGroup &group : _paths)
    {
        if(group.size() < max) max = group.size();
    }

    return max;
}

void HeatMapOptions::refresh()
{
    loadOptions();
}
