//
// Created by romain on 20/04/2026.
//

#include "RotamerMenu.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>

RotamerMenu::RotamerMenu(Scene *prev) : ListView(prev)
{

}

RotamerMenu::~RotamerMenu()
{
}

void RotamerMenu::setup() {
    addTitle("Rotamer Menu");
    ListView::setup();
}

void RotamerMenu::buttonPressed(std::string tag, Button *button)
{
    Scene::buttonPressed(tag, button);

    ListView::buttonPressed(tag, button);

}

Renderable *RotamerMenu::getLine(int i)
{
    TextButton *tb = nullptr;
    if (i == lineCount() - 1)
    {
        tb = new TextButton("List all the rotamers", this);
    }
    return tb;
}

size_t RotamerMenu::lineCount()
{
    return 1;
}
