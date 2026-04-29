//
// Created by romain on 24/04/2026.
//

#include "RotamerView.h"
#include <vagabond/core/Model.h>
#include "vagabond/core/Chain.h"
#include <vagabond/core/ModelManager.h>
#include <vagabond/core/EntityManager.h>




RotamerView::RotamerView(Scene *prev, std::string modelName, Chain *chain) : Scene(prev),  _modelName(modelName), _chain(chain)
{
    std::cout << "qwak\n";

}

RotamerView::~RotamerView()
{
    //previous();
    _model->unload();
    std::cout << "model unloaded\n";

}
void RotamerView::setup()
{
    addTitle ("Rotamer View");
    //showChain();
   // std::cout << "RotamerView::setup() end\n";
    viewModel(/*std::string modelName, Chain *chain*/);

}

/*void RotamerView::buttonPressed(std::string tag, Button *button)
{
    Scene::buttonPressed(tag, button);
    std::cout << "boom\n";

}*/

/*void RotamerView::showChain(Model *model, Chain *chain)
{
    /*std::cout << "Pol\n";
    Display *d = new Display(this);
    DisplayUnit *unit = new DisplayUnit(d);
    std::cout << "display " << _iddd << '\n';
    unit->setOwnsAtoms();
    unit->loadAtoms(_group->chain(_iddd));
    unit->displayAtoms(false, false);
    d->show();
    std::cout << "qwak\n";
}*/
void RotamerView::loadModelChain(Model *model, Chain *chain, DisplayUnit *unit)
{
    //loadModelChain function
    model->polymerFromChain(chain)->load();
    unit->loadAtoms(model->polymerFromChain(chain)->currentAtoms());
    unit->displayAtoms(false, false);
    unit->startWatch();
    //std::cout << "BOOM\n";
}

void RotamerView::viewModel(/*std::string modelName, Chain *chain*/)
{

    //chainAssignement ViewModel
    ModelManager *mm = ModelManager::manager();
    _model = mm->model(_modelName);

    Display *d = new Display(this);
    DisplayUnit *unit = new DisplayUnit(d);
    unit->setOwnsAtoms();
    loadModelChain(_model, _chain,unit);
    d->addDisplayUnit(unit);
    d->show();

    //std::cout << "boop\n";
}