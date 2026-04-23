//
// Created by romain on 20/04/2026.
//

#include "RotamerMenu.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/EntityManager.h>


RotamerMenu::RotamerMenu(Scene *prev) : Scene(prev)
{

}

RotamerMenu::~RotamerMenu()
{
}

void RotamerMenu::setup()
{
    addTitle("Rotamer Menu");
}

void RotamerMenu::buttonPressed(std::string tag, Button *button)
{
    Scene::buttonPressed(tag, button);
    showBackButton();
}
