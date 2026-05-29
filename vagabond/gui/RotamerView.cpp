//
// Created by romain on 24/04/2026.
//

#include "RotamerView.h"
#include "vagabond/gui/elements/Slider.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/Rotamers.h>
#include <vagabond/core/RotamerModifier.h>
#include <vagabond/core/ModelManager.h>
#include <vagabond/gui/elements/TextButton.h>




RotamerView::RotamerView(Scene *prev, std::string modelName, Instance *inst)
:  Scene(prev), Display(prev), _inst(inst), _modelName(modelName)
{
    _inst->load();
    _modifier = new RotamerModifier(_inst);
}

RotamerView::~RotamerView()
{
    _inst->unload();
    std::cout << "model unloaded\n";
    delete _modifier;
}
void RotamerView::setup()
{
    addTitle ("Rotamer View");
}

void RotamerView::buttonPressed(std::string tag, Button *button)
{
    if (tag == "zero")
    {
        _modifier->resetRotamers();
    }
}

void RotamerView::loadModelChain(Model *model, Instance *inst, DisplayUnit *unit)
{
    unit->loadAtoms(inst->currentAtoms());
    unit->displayAtoms(false, false);
    unit->startWatch();
}

void RotamerView::viewModel()
{
    ModelManager *mm = ModelManager::manager();
    _model = mm->model(_modelName);

    DisplayUnit *unit = new DisplayUnit(this);
    unit->setOwnsAtoms();
    loadModelChain(_model, _inst,unit);
    setupSlider();
    addDisplayUnit(unit);
}

void RotamerView::rotaList()
{
    RotamerLibrary();
}

void RotamerView::finishedDragging(std::string tag, double x, double y)
{
    _modifier->submitJobAndRetrieve(x);
}

void RotamerView::setupSlider()
{
    removeObject(_rangeSlider);
    delete _rangeSlider;
    Slider *s = new Slider();
    s->setDragResponder(this);
    s->resize(0.5);
    s->setup("Rotamer selection", _min, _max, _step);
    s->setStart(0.5, 0.);
    s->setCentre(0.5, 0.85);
    _rangeSlider = s;
    addObject(s);
    {
        TextButton *t = new TextButton("Initial rotamers", this);
        t->setRight(0.8, 0.8);
        t->setReturnTag("zero");
        addObject(t);
    }
}