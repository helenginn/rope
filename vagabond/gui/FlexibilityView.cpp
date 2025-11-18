#include "FlexibilityView.h"
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/HBondMenu.h>
#include <vagabond/gui/Atom2AtomExplorer.h>

#include <vagabond/core/Instance.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/ResidueId.h>
#include <vagabond/core/HBondManager.h>
#include <vagabond/core/RAMovement.h>
#include <vagabond/core/Atom3DPosition.h>
#include <vagabond/utils/Eigen/Dense>




using Eigen::Matrix3f;



FlexibilityView::FlexibilityView(Scene *prev, Instance *inst, Flexibility *flex)
: Scene(prev), Display(prev)
{
	_flex = flex;
	_instance = inst;
	_instance->load();
	_controller = new FlexibilityController(this, _flex, _instance);
	setPingPong(true);
}

FlexibilityView::~FlexibilityView()
{
	stopGui();
}

void FlexibilityView::makeMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setReturnTag("menu");
	text->setRight(0.95, 0.1);
	addObject(text);
}

void FlexibilityView::buttonPressed(std::string tag, Button *button)
{

    if (_controller && _controller->handleButton(tag, button))
        return;

	if (tag == "menu")
	{
		_controller->showMenu(button);
		return;
	}

	else if (_controller->handleButton(tag, button))
	{
		return; 
	}
		
	Display::buttonPressed(tag, button);
}


void FlexibilityView::showCloud(DisplayUnit *unit, AtomGroup *grp)
{	
	unit->loadAtoms(grp);
	_unit->disableUnit(true);
	unit->displayAtoms(false, true);
	unit->setMultiBondMode(true);
	unit->startWatch();
	addDisplayUnit(unit);
}



void FlexibilityView::reset()
{
    // Clear hydrogen bond pairs
    _hBondPairs.clear();
    
    // Reset selection flag
    _selectFlag = false;
    if (_flex)
    {
        _flex->clearHBonds();
    }
}

void FlexibilityView::setup()
{
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();
	_unit = new DisplayUnit(this);
	_unit->loadAtoms(grp, _instance->entity());
	_unit->displayAtoms();
	_unit->startWatch();
	addDisplayUnit(_unit);

	Display::setup();
	_flex->prepareResources();
	// return to main menu of FlexibilityView
	setupSlider();
	_flex->submitJobAndRetrieve(0.0);
	makeMenu();
}

void FlexibilityView::checkHBondSelection()
{
    if (_selectFlag) {
       	callAddHBonds(_hBondPairs);
        // _flex->processMultipleHBonds();
    }
}

void FlexibilityView::callAddHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs) 
{
	for (auto &pair : donorAcceptorPairs) 
	{
		std::cout << "Calling callAddHbonds..." << std::endl;
    	_flex->addHBond(pair);
    }
    _flex->addVnWBond();
}

void FlexibilityView::setupSlider()
{
	removeObject(_rangeSlider);
	delete _rangeSlider;
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Flexibility amplifier", _min*10, _max*10, _step);
	s->setStart(0.5, 0.);
	s->setCentre(0.5, 0.85);
	_rangeSlider = s;
	addObject(s);

}

void FlexibilityView::finishedDragging(std::string tag, double x, double y)
{
	float num = x / 1.;
	float test_retrival = _flex->submitJobAndRetrieve(num);
	_first = false;
} 

// add button text to slider that gets you to atom2artoExpolorer
// fo rthis i will need: scene (this), _instance, RAMovent
void FlexibilityView::openAtom2AtomExplorer()
{
	_flex->submitJobAndRetrieve(0.5f);
	std::vector<Atom3DPosition> list; // or should it be std::vector<Atom3DPosition*> list?
	std::vector<Posular> disVec;

	if (!_instance->hasSequence())
	{
		return; // maybe with error message
	}

	AtomGroup *grp = _instance->currentAtoms();
	Sequence *seq = static_cast<Polymer *>(_instance)->sequence();
	disVec.reserve(seq->size());
	const AtomVector &atoms = grp->atomVector();
	for (Atom *atom : atoms)
	{
		if (!atom->isReporterAtom()){ continue; }
		const ResidueId &id = atom->residueId();
		Residue *local = seq->residue(id);
		Residue *res = seq->master_residue(local);

		Atom3DPosition a3Dp(res, atom->atomName()); // is this ok? or should it be the Atom3DPosition a3Dp = new Atom3DPosition(res, "CA")?
		a3Dp.setEntity(_instance->entity());
		list.push_back(a3Dp);

		glm::vec3 curPos = atom->derivedPosition();
        glm::vec3 initPos = atom->otherPosition("other");

        glm::vec3 displacement = curPos - initPos;
        disVec.push_back(displacement);
        std::cout << atom->desc() << " " << displacement << " " << a3Dp << std::endl;

	}

	std::cout << "[debug_openAtom2AtomExplorer] Greetings from openAtom2AtomExplorer!" << std::endl;
	std::cout << list.size() << std::endl;
	RAMovement movement = RAMovement::movements_from(list, disVec);
	Atom2AtomExplorer *a2a = new Atom2AtomExplorer(this, _instance, movement);
	a2a->show();
}



