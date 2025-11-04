#include "FlexibilityView.h"
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskForRange.h>
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
		m->addOption("Save sampled structures", "save_samples");
		m->addOption("Explore distance matrix", "dist_matrix");
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
	openAtom2AtomExplorer();
	if (!_selectFlag)
	{
		BadChoice *bch = new BadChoice(this, "Please select hbonds first and then come back for the B-factors");
		setModal(bch);
	}
	else
	{
		// Start the range input sequence
		AskForText *aft = new AskForText(this, "Enter minimum range (default 0):",
		                                 "range_min", this, TextEntry::Numeric);
		setModal(aft);
	}
}
else if (tag == "range_min")
{
	TextEntry *te = static_cast<TextEntry *>(button);
	float min = atof(te->scratch().c_str());
	if (min != min || !isfinite(min))
	{
		min = 0;
	}
	_minRange = min;

	// Now ask for the max
	AskForText *aft = new AskForText(this, "Enter maximum range (default num of V columns):",
	                                 "range_max", this, TextEntry::Numeric);
	setModal(aft);
}
else if (tag == "range_max")
{
	TextEntry *te = static_cast<TextEntry *>(button);
	std::string text = te->scratch();
	float max = _flex->getVcolumns();

	if (!text.empty())
	{
		max = atof(text.c_str());
		if (max != max || !isfinite(max))
		{
			max = _flex->getVcolumns();  // fallback if garbage typed
		}
	}
	_maxRange = max;

	// Perform calculation
	_flex->setColRangeUser(_minRange, _maxRange-1);
	_flex->generateAtomCloud();
	_flex->calculateFreeEnergy();

	std::string flexTag = _flex->getFlexTag();
	DisplayUnit *unitCloud = new DisplayUnit(this);
	AtomGroup *grp = _instance->currentAtoms();
	const AtomVector &atoms = grp->atomVector();
	for (Atom *atom : atoms)
	{
		WithPos pos = atom->otherPositions(flexTag);
		atom->setDerivedPositions(pos);
		Matrix3f cov = atom->otherAnisoBfactors(flexTag);
	}
	showCloud(unitCloud, grp);
}
else if (tag == "options_save_samples")
{
	if (!_selectFlag)
	{
		BadChoice *bch = new BadChoice(this, "Please select hbonds first and then come back for the B-factors");
		setModal(bch);
	}
	else
	{
		AskForText *aft = new AskForText(this, "How many samples strcutures you want to save?:",
		                                 "num_samples", this, TextEntry::Numeric);
		setModal(aft);
	}
}
else if (tag == "num_samples")
{
	TextEntry *te = static_cast<TextEntry *>(button);
	float numSample = atof(te->scratch().c_str());
	if (numSample != numSample || !isfinite(numSample) || numSample <= 0)
	{
		numSample = 1;
	}
	_numSample = static_cast<int>(numSample);
	_flex->setNumSamples(_numSample);
	// _flex->generateAtomCloud();
	double lambda = 0.5;
	_flex->saveSampledStructures(_numSample, "sample_structure", lambda);
}
else if (tag == "options_dist_matrix")
{
	openAtom2AtomExplorer();
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
    // _flex->setColRange(10, true);
	_flex->processMultipleHBonds();


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
        _flex->processMultipleHBonds();
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
	AtomGroup *grp = _instance->currentAtoms();
	disVec.reserve(grp->sequence()->size());
	const AtomVector &atoms = grp->atomVector();
	for (Atom *atom : atoms)
	{
		if (!atom->isReporterAtom()){ continue; }
		const ResidueId &id = atom->residueId();
		Residue *res = grp->sequence()->residueLike(id);

		Atom3DPosition a3Dp(res, atom->atomName()); // is this ok? or should it be the Atom3DPosition a3Dp = new Atom3DPosition(res, "CA")?
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



