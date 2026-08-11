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
        TextButton *n = new TextButton("RotaMap", this);
        n->setRight(0.2, 0.9);
        n->setReturnTag("rotaMap");
        addObject(n);
    }
    {
        _line = new Line();
        _line2 = new Line();
        _line3 = new Line();
        _line4 = new Line();
        _line5 = new Line();
        _line6 = new Line();
        _line7 = new Line();


        addObject(_line);
        addObject(_line2);
        addObject(_line3);
        addObject(_line4);
        addObject(_line5);
        addObject(_line6);
        addObject(_line7);
    }
}

void RotamerView::buttonPressed(std::string tag, Button *button)
{
    if (tag == "zero")
    {
        _modifier->submitJobAndRetrieve(2.f, RotamerModifier::Reset);
    }
    if (tag == "save") // saving current structure
    {
        _line4->clearVertices();
        _line5->clearVertices();
        _line4->addPoint(_modifier->axisForChain(RotamerModifier::Start, "A"));
        _line4->addPoint(_modifier->axisForChain(RotamerModifier::End, "A"));
        _line5->addPoint(_modifier->axisForChain(RotamerModifier::Start, "B"));
        _line5->addPoint(_modifier->axisForChain(RotamerModifier::End, "B"));


    }
    if (tag == "rotaMap")
    {
        _modifier->generateRotamerMapPosition();
        std::cout << "beep" << std::endl;
    }
    if (tag == "axis")
    {
        std::vector<glm::vec3> points = _modifier->drawAxis();
        _line3->clearVertices();
        for (auto point : points)
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
    std::vector<std::vector<glm::vec3>> drawing {};
    drawing = _modifier->getVertices();
    _line->clearVertices();

    for (auto boxes: drawing)
    {
        for (auto point : boxes)
        {
            _line->addPoint(point);
        }
    }
    _line->forceRender();
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