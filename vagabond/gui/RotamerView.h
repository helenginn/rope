//
// Created by romain on 24/04/2026.
//

#ifndef __vagabond__RotamerView__
#define __vagabond__RotamerView__
#include "Display.h"
#include <vagabond/core/Chain.h>

class Model;

class RotamerView : public Scene                                                    //NOT CRASHING ANYMORE, BUT NEED TO CONVERT TO DISPLAY UNIT
{
public:
    RotamerView(Scene *prev, std::string modelName, Chain *chain);//, AtomContent *content, std::string chainName);
    virtual ~RotamerView();
    void setup();
    //void showChain(Model *model, Chain *chain);
    //virtual void buttonPressed(std::string tag, Button *button);
    void viewModel(/*std::string modelName, Chain *chain*/);
    void loadModelChain(Model *model, Chain *chain, DisplayUnit *unit);

private:
    Model *_model = nullptr;
    Chain *_chain = nullptr;
    std::string _modelName ={} ;
};
#endif
