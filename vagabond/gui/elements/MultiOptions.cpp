#include "MultiOptions.h"
#include "AskYesNo.h"
#include "ChooseRange.h"

MultiOptions::MultiOptions(Scene *scene, std::string text, std::string tag, ButtonResponder *sender) : Modal(scene, 0.6, 0.4)
{
    Text *t = new Text(text);
    t->resize(0.8);
    t->setCentre(0.5, 0.35);
    addObject(t);

    _sender = sender;
    _tag = tag;

    setDismissable(true);
}

void MultiOptions::addYesNo(std::string ynText)
{
    AskYesNo *ayn = new AskYesNo(this, std::string ynText, "mist", sender);
}

void MultiOptions::addSlider(std::string sliderText, float min, float max)
{
    ChooseRange *cr = new ChooseRange(this, sliderText, tag + "_num_paths", sender);
    cr->setDefault(min);
    cr->setRange(min, max, min-max);
}
