//
// Created by romain on 20/04/2026.
//

#include "RotamerMenu.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/EntityManager.h>


RotamerMenu::RotamerMenu(Scene *prev) : ModelMenu(prev)
{

}

RotamerMenu::~RotamerMenu()
{
}

void RotamerMenu::setup()
{
    ModelMenu::setup();
    addTitle("zboui");


}

void RotamerMenu::buttonPressed(std::string tag, Button *button)
{
    ModelMenu::buttonPressed(tag, button);
    //showBackButton();
}
