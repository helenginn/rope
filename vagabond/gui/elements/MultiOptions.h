#ifndef __vagabond__MultiOptions__
#define __vagabond__MultiOptions__

#include "Modal.h"
#include "ButtonResponder.h"

class MultiOptions : public Modal
{
public:
    MultiOptions(Scene *scene, std::string text, std::string tag, ButtonResponder *sender);

    void addYesNo(std::string ynText);

    void addSlider(std::string sliderText, float min, float max);

    virtual void buttonPressed(std::string tag, Button *button);

private:
    ButtonResponder *_sender;
    std::string _tag;
};

#endif     
