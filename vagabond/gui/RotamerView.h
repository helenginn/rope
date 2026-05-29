//
// Created by romain on 24/04/2026.
//

#ifndef __vagabond__RotamerView__
#define __vagabond__RotamerView__
#include "Display.h"
#include <vagabond/gui/elements/DragResponder.h>



class Model;
class Instance;
class Slider;
class RotamerModifier;
class RotamerView : public Display,  public DragResponder                                           //NOT CRASHING ANYMORE, BUT NEED TO CONVERT TO DISPLAY UNIT
{
public:
    RotamerView(Scene *prev, std::string modelName, Instance *inst);//, AtomContent *content, std::string chainName);
    virtual ~RotamerView();
    void setup();
    //void showChain(Model *model, Chain *chain);
    //virtual void buttonPressed(std::string tag, Button *button);
    void viewModel();
    void loadModelChain(Model *model, Instance *inst, DisplayUnit *unit);
    void rotaList();
    void setupSlider();
    void buttonPressed(std::string tag, Button *button);
    virtual void finishedDragging(std::string tag, double x, double y);
    //void submitJobAndRetrieve(int sliderValue);
    //void submitJob(int sliderValue);
protected:
    //virtual void prepareResources();
private:

    Slider *_rangeSlider = nullptr;
    bool _first = true;
    double _min = 0;
    double _max = 1;
    double _step = 0.05;
    Model *_model = nullptr;
    Instance *_inst = nullptr;
    std::string _modelName ={} ;
    RotamerModifier *_modifier = {};

};
#endif
