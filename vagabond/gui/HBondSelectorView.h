#ifndef __vagabond__HBondSelectorView__
#define __vagabond__HBondSelectorView__

#include <vagabond/core/Flexibility.h>
#include <vagabond/gui/elements/ListView.h>
#include <map>

class TickBoxes;
class GuiBalls;

class HBondSelectorView : public ListView
{
public:
    HBondSelectorView(Scene *prev, 
                      const std::vector<Flexibility::HBondEntity> &hbonds,
                      GuiBalls *balls);
    
    virtual void setup();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual size_t lineCount();
    virtual Renderable *getLine(int i);

private:
    const std::vector<Flexibility::HBondEntity> &_hbonds;
    GuiBalls *_balls;
    TickBoxes *_allTickBoxes = nullptr;
    std::set<int> *_selectedIndices = nullptr;
    std::map<int, bool> _tickStates;
};

#endif