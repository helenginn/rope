//
// Created by romain on 20/04/2026.
//

#ifndef __vagabond__RotamerMenu__
#define __vagabond__RotamerMenu__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/gui/elements/Scene.h>

class RotamerMenu : public ListView
{
public:
    RotamerMenu(Scene *prev);
    virtual ~RotamerMenu();
    virtual void setup();

    virtual size_t lineCount();
    virtual Renderable *getLine(int i);
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual void respond();
    virtual void refresh();
private:
};








#endif
