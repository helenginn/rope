#ifndef __vagabond__HBondSelectorView__
#define __vagabond__HBondSelectorView__

#include <vagabond/core/Flexibility.h>
#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/FlexibilityTypes.h>
#include <map>
#include <functional>

class TickBoxes;
class GuiBalls;

class HBondSelectorView : public ListView
{
public:
    HBondSelectorView(Scene *prev, 
                      const std::vector<HBondEntity> &hbonds,
                      GuiBalls *balls);
    
    virtual void setup();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);
    virtual size_t lineCount();
    virtual Renderable *getLine(int i);

    using ApplyCallback = std::function<void(const std::vector<int> &selectedIndices)>;

    void setApplyCallback(ApplyCallback callback)
    {
        _applyCallback = callback;
    }
private:
    ApplyCallback _applyCallback = nullptr;
    const std::vector<HBondEntity> &_hbonds;
    GuiBalls *_balls;
    TickBoxes *_allTickBoxes = nullptr;
    std::set<int> *_selectedIndices = nullptr;
    std::map<int, bool> _tickStates;
};

#endif