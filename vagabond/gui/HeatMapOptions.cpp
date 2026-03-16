#include <vagabond/gui/HeatMapOptions.h>
#include <vagabond/gui/HeatMapView.h>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/Entity.h>

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>

#include "VagWindow.h"

HeatMapOptions::HeatMapOptions(Scene *prev, Entity *entity) : Scene(prev), _entity(entity)
{
    _entity = entity;

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
        Slider *sPaths = new Slider();
        sPaths->setDragResponder(this);
        sPaths->resize(0.5);
        sPaths->setup("Number of paths", 2, 11, 1);
        sPaths->setReturnTag("paths");
	    sPaths->setCentre(0.7, 0.35);
        addObject(sPaths);
    }

    {
        Slider *sNN = new Slider();
        sNN->setDragResponder(this);
        sNN->resize(0.5);
        sNN->setup("Number of nearest neighbours", 2, 10, 1);
        sNN->setReturnTag("neighbours");
	    sNN->setCentre(0.7, 0.5);
        addObject(sNN);
    }

    {
        Slider *sTime = new Slider();
        sTime->setDragResponder(this);
        sTime->resize(0.5);
        sTime->setup("Number of time divisions", 1, 10, 1);
        sTime->setReturnTag("timepoints");
	    sTime->setCentre(0.7, 0.65);
        addObject(sTime);
    }

    {
        TickBoxes *tb = new TickBoxes(this, this);
        tb->addOption("mist");
        tb->arrange(0.4, 0.22, 0.9, 0.5);
        addObject(tb);
    }

    float bottom = 0.9;

    {
	    TextButton *t = new TextButton("Generate heat map", this);
	    t->setRight(0.8, bottom);
	    t->setReturnTag("heatmap");
	    addObject(t);
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
        std::vector<struct EntropyForHeatMap> entropyData = {};
        
        std::cout << "adding job" << std::endl;

        std::function<void(std::vector<EntropyForHeatMap> &)> callback;
	callback = [this](std::vector<EntropyForHeatMap> &entropyData)
	{
	    addMainThreadJob([this, entropyData]()
	    {
		HeatMapView *view = new HeatMapView(this, entropyData);
		view->show();
	    });
            
	    showBackButton();
	};
           
	Environment::pathManager()->setEntropyCallback(callback);
        
        hideBackButton();

        prepareProgress(_entity->instanceCount()-1, "Calculating path entropy...");

        VagWindow::addJob("path-entropy=" + _entity->name() + "," + std::to_string(_flagPar.nf) + "," + std::to_string( _flagPar.timeDivisions));
 
	return;
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

void HeatMapOptions::prepareProgress(int ticks, std::string text)
{
    VagWindow::window()->requestProgressBar(ticks, text);
}
