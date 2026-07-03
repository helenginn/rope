#include "HBondSelectorView.h"
#include "GuiBalls.h"
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>

HBondSelectorView::HBondSelectorView(Scene *prev,
                                     const std::vector<HBondEntity> &hbonds,
                                     GuiBalls *balls)
: ListView(prev), _hbonds(hbonds), _balls(balls)
{

     // initially all is unticked
    for (size_t i = 0; i < _hbonds.size(); i++)
    {
        _tickStates[i] = false;
    }
}

void HBondSelectorView::setup()
{
    addTitle("Select bonds to highlight");
    
    // Select All button
    {
        TextButton *selectAll = new TextButton("Select All", this);
        selectAll->setReturnTag("select_all");
        selectAll->setLeft(0.15, 0.20);
        addObject(selectAll);
    }
    
    // Clear All button
    {
        TextButton *clearAll = new TextButton("Clear All", this);
        clearAll->setReturnTag("clear_all");
        clearAll->setLeft(0.35, 0.20);
        addObject(clearAll);
    }
    

    // Apply button
    {
        TextButton *apply = new TextButton("Apply", this);
        apply->setReturnTag("apply");
        apply->setCentre(0.7, 0.20);
        addObject(apply);
    }
    ListView::setup();
}

size_t HBondSelectorView::lineCount()
{
    return _hbonds.size();
}


Renderable *HBondSelectorView::getLine(int i)
{

    const auto &hbe = _hbonds[i];
    std::string label = hbe.Hydrogen->desc() + " -> " + hbe.Acceptor->desc();
    
    Box *b = new Box();
    {
        std::string image = _tickStates[i] ? 
            "assets/images/tickbox_filled.png" : 
            "assets/images/tickbox_empty.png";
        
        ImageButton *tick = new ImageButton(image, this);
        tick->setLeft(0.0, 0.0);
        tick->resize(0.05);
        tick->setReturnTag("toggle_" + std::to_string(i));
        b->addObject(tick);
    }

    {
        TextButton *text = new TextButton(label, this);
        text->setLeft(0.05, 0.0);
        text->setReturnTag("toggle_" + std::to_string(i));
        b->addObject(text);
    }
    
    return b;
}



void HBondSelectorView::buttonPressed(std::string tag, Button *button)
{
    std::string toggle_idx = Button::tagEnd(tag, "toggle_");
    if (toggle_idx.length() > 0)
    {
        int idx = atoi(toggle_idx.c_str());
        _tickStates[idx] = !_tickStates[idx];
        refresh();
        return;
    }
    
    if (tag == "select_all")
    {
        for (size_t i = 0; i < _hbonds.size(); i++)
        {
            _tickStates[i] = true;
        }
        return;
    }
    
    if (tag == "clear_all")
    {
        for (size_t i = 0; i < _hbonds.size(); i++)
        {
            _tickStates[i] = false;
        }
        refresh();
        return;
    }
    
    if (tag == "apply")
    {
        std::vector<int> selected;
        for (size_t i = 0; i < _hbonds.size(); i++)
        {
            if (_tickStates[i])
            {
                selected.push_back(i);
            }
        }
        back();

        if (_applyCallback)
        {
            _applyCallback(selected);
        }
        else if (_balls)
        {
            // fallback: original behaviour for FlexibilityView
            _balls->clearAtomHighlights(GuiBalls::HBond);
            for (int i : selected)
            {
                const auto &hbe = _hbonds[i];
                _balls->highlightAtom(hbe.Hydrogen, GuiBalls::HBond);
                _balls->highlightAtom(hbe.Acceptor, GuiBalls::HBond);
            }
        }
        
        return;
    }
    
    ListView::buttonPressed(tag, button);
}
