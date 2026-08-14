//
// Created by romain on 24/04/2026.
//

#include "RotamerView.h"
#include "vagabond/gui/elements/Slider.h"
#include <vagabond/core/Model.h>
#include <../core/rotamers/Rotamers.h>
#include <vagabond/core/rotamers/RotamerModifier.h>
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
    {
        TextButton *t = new TextButton("hide/show collisionBoxes" , this);
        t->setRight(0.9, 0.9);
        t->setReturnTag("collision");
        addObject(t);
    }
    {
        _line = new Line();
        _line2 = new Line();
        _line3 = new Line();
        _line4 = new Line();
        _line5 = new Line();
        _line6 = new Line();
        _line7 = new Line();
        _para = new Parallelepiped();

        addObject(_line);
        addObject(_line2);
        addObject(_line3);
        addObject(_line4);
        addObject(_line5);
        addObject(_line6);
        addObject(_line7);
        addObject(_para);
        _line2->setColour(0.2, 0.9, 0.3);
    }
}

void RotamerView::buttonPressed(std::string tag, Button *button)
{
    if (tag == "zero")
    {
        _modifier->submitJobAndRetrieve(2.f, RotamerModifier::Reset);
    }
    if (tag == "collision") // saving current structure
    {
        if (_collision)
        {
            _collision = false;
        }
        else
        {
            _collision = true;
        }
    }
    if (tag == "axis")
    {
        std::vector<glm::vec3> const points = _modifier->drawAxis();
        _line3->clearVertices();
        for (const glm::vec3 point : points)
            _line3->addPoint(point);
        _line3->forceRender();
    }
    Scene::buttonPressed(tag, button);
}

void RotamerView::loadModelChain( Instance *inst, DisplayUnit *unit)
{
    unit->loadAtoms(inst->currentAtoms());
    unit->displayAtoms(false, false);
    unit->startWatch();
}

void RotamerView::viewModel()
{
    DisplayUnit *unit = new DisplayUnit(this);
    loadModelChain( _inst,unit);
    unit->setMultiBondMode(true);
    setupSlider();
    addDisplayUnit(unit);
}

void RotamerView::rotaList()
{
    RotamerLibrary();
}

void RotamerView::finishedDragging(std::string tag, double x, double y)
{
    if (tag == "X")
    {
        _modifier->move(x,RotamerModifier::MoveX);
    }
    if (tag == "Y")
    {
        _modifier->move(x,RotamerModifier::MoveY);
    }
    if (_collision)
    {
        std::vector<std::pair<glm::vec3,glm::vec3>> drawing {};
        drawing = _modifier->getVertices();
        _para->clearVertices();
        for (auto pair : drawing)
        {
            _para->addParallelepiped(pair.first,pair.second);
        }
        _para->setAlpha(1.0f);
        _para-> forceRender();
    }
    _line2->clearVertices();
    std::vector<glm::vec3> axePoints = _modifier->drawAxis();
    _line2->addPoint(axePoints[0]);
    _line2->addPoint(axePoints[1]);
    _line2->addPoint(axePoints[2], false);
    _line2->addPoint(axePoints[3]);
    _line2->forceRender();
}

void RotamerView::setupSlider()
{
    removeObject(_rangeSlider);
    delete _rangeSlider;
    Slider *s = new Slider();

    s->setDragResponder(this);
    s->resize(0.5);
    s->setup("Rotamer selection", _min, _max, _step);
    s->setStart(0.5, 0);
    s->setCentre(0.5, 0.85);
    s->setReturnTag("X");
    _rangeSlider = s;
    addObject(s);

    removeObject(_rangeSlider2);
    delete _rangeSlider2;
    Slider *s2 = new Slider();
    s2->setVertical(true);
    s2->setDragResponder(this);
    s2->resize(0.5);
    s2->setup("", _min, _max, _step);
    s2->setStart(0, 0.5);
    s2->setCentre(0.2, 0.6);
    s2->setReturnTag("Y");

    _rangeSlider2 = s2;
    addObject(s2);

}