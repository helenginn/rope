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
    {
        TextButton *t = new TextButton("Initial rotamers", this);
        t->setRight(0.8, 0.7);
        t->setReturnTag("zero");
        addObject(t);
    }
    {
        TextButton *t = new TextButton("Save position to RTL4-" + std::to_string(_number) , this);
        t->setRight(0.9, 0.9);
        t->setReturnTag("save");
        addObject(t);
    }
    {
        TextButton *n = new TextButton("RotaMap", this);
        n->setRight(0.2, 0.9);
        n->setReturnTag("rotaMap");
        addObject(n);
    }
    {
        TextButton *n = new TextButton("axis", this);
        n->setRight(0.2, 0.5);
        n->setReturnTag("axis");
        addObject(n);
    }
    {
        _line = new Line();
        _line2 = new Line();
        _line3 = new Line();
        _line4 = new Line();
        _line5 = new Line();

        addObject(_line);
        addObject(_line2);
        addObject(_line3);
        addObject(_line4);
        addObject(_line5);
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
        // _line->clearVertices();
        // _line2->clearVertices();
        // _line->addPoint(_modifier->axisForChain(RotamerModifier::Start, "A"));
        // _line->addPoint(_modifier->axisForChain(RotamerModifier::End, "A"));
        // //_line->forceRender();
        // _line2->addPoint(_modifier->axisForChain(RotamerModifier::Start, "B"));
        // _line2->addPoint(_modifier->axisForChain(RotamerModifier::End, "B"));
        // //_line2->forceRender();


    }
    if (tag == "rotaMap")
    {
        _modifier->generateRotamerMapPosition();
        std::cout << "beep" << std::endl;
    }
    if (tag == "axis")
    {
        std::vector<glm::vec3> axis = _modifier->makePlan();
        glm::vec3 startingPoint =_modifier->axisForChain(RotamerModifier::Start, "A");
        _line3->addPoint(startingPoint);
        _line4->addPoint(startingPoint);
        _line5->addPoint(startingPoint);
        _line3->addPoint(startingPoint+axis[0]*glm::vec3(3.0));
        _line4->addPoint(startingPoint+axis[1]*glm::vec3(3.0));
        _line5->addPoint(startingPoint+axis[2]*glm::vec3(3.0));
        _line3->forceRender();
        _line4->forceRender();
        _line5->forceRender();


       // _modifier->axisForChain(RotamerModifier::Start, "A")+_modifier->axisForChain(RotamerModifier::End, "A"))/glm::vec3(2.0)
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
        _modifier->submitJobAndRetrieve(x, RotamerModifier::MoveX);
    }
    if (tag == "Y")
    {
        _modifier->submitJobAndRetrieve(x, RotamerModifier::MoveY);
    }
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