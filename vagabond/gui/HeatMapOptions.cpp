#include <vagabond/gui/HeatMapOptions.h>
#include <vagabond/gui/HeatMapView.h>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/Entity.h>

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>

#include "VagWindow.h"

HeatMapOptions::HeatMapOptions(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths) : Scene(prev)
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

    PathManager::GroupedMap map = Environment::pathManager()->groupedPathsForEntity(_entity);

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
        sPaths->setup("Number of paths", 2, 20, 1);
        sPaths->setReturnTag("paths");
	    sPaths->setCentre(0.7, 0.35);
        addTempObject(sPaths);
    }

    {
        Slider *sNN = new Slider();
        sNN->setDragResponder(this);
        sNN->resize(0.5);
        sNN->setup("Number of nearest neighbours", 2, 20, 1);
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
        struct FlagParameters options = _flagPar;

        struct EntropyForHeatMap entropyData = {};

        std::function<void(EntropyForHeatMap &)> callback;
		callback = [this, button](EntropyForHeatMap &entropyData)
		{
			addMainThreadJob([this, entropyData, button]()
			{
			HeatMapView *view = new HeatMapView(this, entropyData);

            showBackButton();
			button->setInert(false);

            view->show();
			});
		};
           
	    Environment::pathManager()->setEntropyCallback(callback);
        
        hideBackButton();
        button->setInert();

        prepareProgress(_entity->instanceCount()-1, "Calculating path entropy...");

        VagWindow::addJob("path-entropy=" + _entity->name() + "," + std::to_string(options.nf) + "," + std::to_string(options.timeDivisions));
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

void HeatMapOptions::refresh()
{
    loadOptions();
}
