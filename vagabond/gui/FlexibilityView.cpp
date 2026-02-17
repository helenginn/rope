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
	_instance = inst;
	_instance->load();
	_controller = new FlexibilityController(this, _instance, flex);
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
	_controller->callPrepareResources();
	// return to main menu of FlexibilityView
	setupSlider();
	_controller->callSubmitJobAndRetrieve(0.0);
	_controller->callSubmitJobAndRetrieve(0.0);
	makeMenu();
}


void FlexibilityView::setupSlider()
{
	removeObject(_rangeSlider);
	delete _rangeSlider;
	Slider *s = new Slider();
	s->setDragResponder(_controller);
	s->resize(0.5);
	s->setup("Flexibility amplifier", _min*10, _max*10, _step);
	s->setStart(0.5, 0.);
	s->setCentre(0.5, 0.85);
	_rangeSlider = s;
	addObject(s);

}


// add button text to slider that gets you to atom2artoExpolorer
// fo rthis i will need: scene (this), _instance, RAMovent
void FlexibilityView::openAtom2AtomExplorer()
{
	// _controller->callSubmitJobAndRetrieve(0.5f);
	std::vector<Atom3DPosition> list; 
	std::vector<Posular> disVec;

	if (!_instance->hasSequence())
	{
		return; // maybe with error message
	}

	AtomGroup *grp = _instance->currentAtoms();
	// grp.printbyname
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


	}

	std::cout << "[debug_openAtom2AtomExplorer] Greetings from openAtom2AtomExplorer!" << std::endl;
	std::cout << list.size() << std::endl;
	std::cout << "[DEBUG disVec] Total atoms: " << disVec.size() << "\n";
	for (size_t i = 0; i < disVec.size(); ++i)
	{
	    const glm::vec3 &d = disVec[i];
	    std::cout << "Atom " << i << ": (" 
	              << d.x << ", " 
	              << d.y << ", " 
	              << d.z << ")\n";
	}
	RAMovement movement = RAMovement::movements_from(list, disVec);
	Atom2AtomExplorer *a2a = new Atom2AtomExplorer(this, _instance, movement);
	a2a->show();
}



