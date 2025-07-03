#include "FlexibilityView.h"

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/HBondMenu.h>

#include <vagabond/core/Instance.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/HBondManager.h>
#include <vagabond/utils/Eigen/Dense>

using Eigen::Matrix3f;



FlexibilityView::FlexibilityView(Scene *prev, Instance *inst, Flexibility *flex)
: Scene(prev), Display(prev)
{
	_flex = flex;
	_instance = inst;
	_instance->load();
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
	if (tag == "menu")
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "options");
		m->addOption("Save state as PDB", "save_state");
		m->addOption("Select h-bonds from file", "selected_hbonds");
		m->addOption("Create B-factor cloud", "bfactor_cloud");
		m->addOption("Clear hydrogen bonds", "clear_hbonds");
		m->setup(c.x, c.y);
		setModal(m);

	}
	else if (tag == "clear_hbonds") // Handle clearing hydrogen bonds
    {
        reset();  // Clear internal state
        _flex->clearHBonds(); // Notify Flexibility to clear bonds
    }
	else if (tag == "options_save_state")
	{
		AskForText *aft = new AskForText(this, "PDB file name to save to:",
		                                 "export_pdb", this);
		setModal(aft);
	}
	else if (tag == "export_pdb")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string filename = te->scratch();

		std::string path = getPath(filename);
		std::string file = getFilename(filename);
		check_path_and_make(path);

		_instance->currentAtoms()->writeToFile(filename);
	}
	else if (tag == "options_selected_hbonds")
	{
		_selectFlag = true;
		HBondMenu *hbmenu = new HBondMenu(this);
		hbmenu->setCallBackFunction([this](std::vector<HBondManager::HBondPair> pairs) 
		{
            _hBondPairs = pairs;
            handleHBonds(_hBondPairs);
       	});

        selectMode(hbmenu, true); // this is neseccary so that the select button appears on screen
		hbmenu->show();
	}
else if (tag == "options_bfactor_cloud")
	{
		if (_selectFlag == true)
		{
			_flex->generateAtomCloud();
			std::string flexTag = _flex->getFlexTag();
			DisplayUnit *unitCloud = new DisplayUnit(this);
			AtomGroup *grp = _instance->currentAtoms();
			const AtomVector &atoms = grp->atomVector();
			// for (Atom *atom : atoms)
			// {
			// 	WithPos pos = atom->otherPositions(flexTag);
			// 	atom->setDerivedPositions(pos);
			// 	Matrix3f cov = atom->otherAnisoBfactors(flexTag);
			// 	atom->setDerivedAnisoBfactors(cov);
			// }
    		showCloud(unitCloud, grp);
		}
		else
		{
			BadChoice *bch = new BadChoice(this, "Please select hbonds first and then come back for the B-factors");
			setModal(bch);
		}
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

void FlexibilityView::handleHBonds(const std::vector<HBondManager::HBondPair>& pairs)
{
    // Add to internal list or perform any other action
    callAddHBonds(pairs);
    _flex->setColIdx(10, false);
	_flex->addMultipleHBonds(pairs);

	// _flex->printHBonds();

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
	// _flex->submitJobAndRetrieve(0.0);
	makeMenu();
	// checkHBondSelection();
}

void FlexibilityView::checkHBondSelection()
{
    if (_selectFlag) {
       	callAddHBonds(_hBondPairs);
        _flex->addMultipleHBonds(_hBondPairs);
    }
}

void FlexibilityView::callAddHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs) 
{
	for (auto &pair : donorAcceptorPairs) 
	{
		std::cout << "Calling callAddHbonds..." << std::endl;
    	_flex->addHBond(pair);
    }
}

void FlexibilityView::setupSlider()
{
	std::cout << "setupSlider called" << std::endl;
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








