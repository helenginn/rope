//
// Created by romain on 24/04/2026.
//

#ifndef __vagabond__RotamerView__
#define __vagabond__RotamerView__
#include "Display.h"
#include <vagabond/gui/elements/DragResponder.h>

#include "elements/Line.h"
#include <vagabond/gui/elements/Parallelepiped.h>


class Model;
class Instance;
class Slider;
class RotamerModifier;
class RotamerView : public Display,  public DragResponder
{
public:
    RotamerView(Scene *prev, std::string modelName, Instance *inst);
    virtual ~RotamerView();
    void setup();
    void viewModel();
    void loadModelChain( Instance *inst, DisplayUnit *unit);
    void rotaList();
    void setupSlider();
    virtual void buttonPressed(std::string tag, Button *button);
    virtual void finishedDragging(std::string tag, double x, double y);
private:

    Slider *_rangeSlider = nullptr;
    Slider *_rangeSlider2 = nullptr;

    bool _first = true;

    double _min = -15;
    double _max = 15;
    double _step = 0.5;
    Model *_model = nullptr;
    Instance *_inst = nullptr;
    std::string _modelName ={} ;
    RotamerModifier *_modifier {};
    int _number {1};
    Parallelepiped * _para {};
    Line *_line = nullptr;
    Line *_line2 = nullptr;
    Line *_line3 = nullptr;
    Line *_line4 = nullptr;
    Line *_line5 = nullptr;
    Line *_line6 = nullptr;
    Line *_line7 = nullptr;
    bool _collision {true};

};
#endif
