#include "VdWSelectorView.h"
#include "GuiBalls.h"
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>

VdWSelectorView::VdWSelectorView(Scene *prev,
                                     const std::vector<Flexibility::VdWBondEntity> &vdwbonds,
                                     GuiBalls *balls)
: ListView(prev), _VdWBonds(vdwbonds), _balls(balls)
{

     // initially all is unticked
    for (size_t i = 0; i < _VdWBonds.size(); i++)
    {
        _tickStates[i] = false;
    }
}

void VdWSelectorView::setup()
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

size_t VdWSelectorView::lineCount()
{
    return _VdWBonds.size();
}


Renderable *VdWSelectorView::getLine(int i)
{

    const auto &vdw = _VdWBonds[i];
    std::string label = vdw.Atom1->desc() + " -> " + vdw.Atom2->desc();
    
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



void VdWSelectorView::buttonPressed(std::string tag, Button *button)
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
        for (size_t i = 0; i < _VdWBonds.size(); i++)
        {
            _tickStates[i] = true;
        }
        return;
    }
    
    if (tag == "clear_all")
    {
        for (size_t i = 0; i < _VdWBonds.size(); i++)
        {
            _tickStates[i] = false;
        }
        refresh();
        return;
    }
    
    if (tag == "apply")
    {
        // Clear old highlights
        _balls->clearAtomHighlights(GuiBalls::VdW);
                
        // Highlight ticked H-bonds
        for (size_t i = 0; i < _VdWBonds.size(); i++)
        {
            if (_tickStates[i])
            {
                const auto &vdw = _VdWBonds[i];
                _balls->highlightAtom(vdw.Atom1, GuiBalls::VdW);
                _balls->highlightAtom(vdw.Atom2, GuiBalls::VdW);
            }
        }
        
        back();
        return;
    }
    
    ListView::buttonPressed(tag, button);
}
