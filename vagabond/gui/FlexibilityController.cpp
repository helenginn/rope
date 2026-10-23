// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If n ot, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "FlexibilityController.h"
#include <vagabond/gui/FlexibilityView.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/FlexSample.h>
#include <vagabond/core/FlexAnalysis.h>
#include <vagabond/core/FlexibilityCache.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Button.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/utils/FileReader.h>



FlexibilityController::FlexibilityController(FlexibilityView *view, Instance *instance, Flexibility *flex)
{
	_view = view;
	_flex = flex;
	_instance = instance;
}

void FlexibilityController::reset()
{
    // Clear hydrogen bond pairs
    _hBondPairs.clear();
    
    // Reset selection flag
    _selectFlag = false;
    if (_flex)
    {
        _flex->clearHBonds();
    }
    _hasSampleWeight = false;
}



void FlexibilityController::showMenu(Button *button)
{
	glm::vec2 c = button->xy();
	Menu *m = new Menu(_view, _view, "options");
	m->addOption("Save state as PDB",      "save_state");
    m->addOption("Select h-bonds from file", "selected_hbonds");
    // m->addOption("Create B-factor cloud",  "bfactor_cloud");
    m->addOption("Clear hydrogen bonds",   "clear_hbonds");
    m->addOption("Save sampled structures","save_samples");
    m->addOption("Explore distance matrix","dist_matrix");
    m->addOption("Select two atoms as targets", "target_atoms");

    m->setup(c.x, c.y);
    _view->setModal(m);
}




bool FlexibilityController::handleButton(const std::string &tag, Button *button)
{

	_tag = tag;
    if (tag == "options_clear_hbonds") { handleClearHBonds(); return true; }
    if (tag == "options_save_state")   { handleSaveState();   return true; }
    if (tag == "export_pdb") { handleExportPDB(button); return true; }
    if (tag == "options_selected_hbonds") { handleSelectedHBonds(button); return true; }
    // if (tag == "options_bfactor_cloud")   { handleBFactorCloud(); return true; }
    if (tag == "range_min")   { handleRangeMin(button); return true; }
    if (tag == "range_max")   { handleRangeMax(button); return true; }
    if (tag == "options_save_samples") { handleSaveSamples(); return true; }
    if (tag == "num_samples")  { handleNumSamples(button); return true; }
    if (tag == "options_dist_matrix") { handleDistMatrix(); return true; }
    if (tag == "options_target_atoms") { handleTargetAtoms(); return true; }
    if (tag == "enter_atomA")   { handleTargetA(button); return true; }
    if (tag == "enter_atomB")   { handleTargetB(button); return true; }
    if (tag == "enter_colIdx")   { handleColumnIdx(button); return true; }



    return false; // FlexibilityView or Display will handle it
}

void FlexibilityController::handleClearHBonds()
{
    reset();
    _flex->clearHBonds();
}

void FlexibilityController::handleSaveState()
{
	AskForText *aft = new AskForText(_view, "PDB file name to save to:",
	                                 "export_pdb", _view);

	_view->setModal(aft);	
}

void FlexibilityController::handleExportPDB(Button *button)
{
    TextEntry *te = static_cast<TextEntry *>(button);
    std::string filename = te->scratch();
    std::cout << "[DEBUG] User entered filename: " << filename << std::endl;
    std::string path = getPath(filename);
    std::string file = getFilename(filename);
    check_path_and_make(path);
    _instance->currentAtoms()->writeToFile(filename);
    std::cout << "[DEBUG] PDB file saved: " << filename << std::endl;

}

void FlexibilityController::handleSelectedHBonds(Button *button)
{

    _selectFlag = true;
    HBondMenu *hbmenu = new HBondMenu(_view);

    hbmenu->setCallBackFunction([this](std::vector<HBondManager::HBondPair> pairs)
    {
        _hBondPairs = pairs;
        callAddHBonds(_hBondPairs);
        // == NEW: set the target coordinate before selecDoFmap ==> if that works, to be moved as a selcetion in the gui
        AtomGroup *atoms = _instance->currentAtoms();
        _flex->selectDoFMap();
        _flex->newJacobian();
        _flex->calculateFlexWeights();
        _flex->calculateTorsionFlexibility();
	    _hasSampleWeight = false;
	    _flex->submitJobAndRetrieve(_currentWeight);
	    });

    _view->selectMode(hbmenu, true);
    hbmenu->show();
}




void FlexibilityController::handleBFactorCloud()
{
	// _view->openAtom2AtomExplorer();
	if (!_selectFlag)
	{
		BadChoice *bch = new BadChoice(_view, "Please select hbonds first and then come back for the B-factors");
		_view->setModal(bch);
	}
	else
	{
		// Start the range input sequence
		AskForText *aft = new AskForText(_view, "Enter minimum range (default 0):",
		                                 "range_min", _view, TextEntry::Numeric);
		_view->setModal(aft);

	}
}

void FlexibilityController::handleRangeMin(Button* button)
{
	TextEntry *te = static_cast<TextEntry *>(button);
	float min = atof(te->scratch().c_str());
	if (min != min || !isfinite(min))
	{
		min = 0;
	}
	_minRange = min;

	// Now ask for the max
	AskForText *aft = new AskForText(_view, "Enter maximum range (default num of V columns):",
	                                 "range_max", _view, TextEntry::Numeric);
	_view->setModal(aft);	
}


void FlexibilityController::handleRangeMax(Button* button)
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
	std::string flexTag = "flexPos";
	FlexAnalysis analysis(_flex, _instance);
	analysis.generateAtomCloud(_minRange, _minRange, flexTag);
	analysis.calculateFreeEnergy();

	
	DisplayUnit *unitCloud = new DisplayUnit(_view);
	AtomGroup *grp = _instance->currentAtoms();
	const AtomVector &atoms = grp->atomVector();
	for (Atom *atom : atoms)
	{
		WithPos pos = atom->otherPositions(flexTag);
		atom->setDerivedPositions(pos);
		Matrix3f cov = atom->otherAnisoBfactors(flexTag);
	}
	_view->showCloud(unitCloud, grp);
}

void FlexibilityController::handleSaveSamples()
{
	// H-bonds actually chosen (the old _selectFlag was set as soon as the menu opened)
	if (_flex->getHBonds().empty())
	{
		BadChoice *bch = new BadChoice(_view, "Please select hbonds first and then come back to save samples");
		_view->setModal(bch);
		return;
	}

	// at weight 0 every "sample" would be the unmoved structure
	if (std::fabs(_currentWeight) < 1e-6f)
	{
		BadChoice *bch = new BadChoice(_view, "The amplitude slider is at 0. Move it before sampling");
		_view->setModal(bch);
		return;
	}

	AskForText *aft = new AskForText(_view, "How many sampled structures you want to save?:",
	                                 "num_samples", _view, TextEntry::Numeric);
	_view->setModal(aft);
}


void FlexibilityController::handleNumSamples(Button* button)
{
	TextEntry *te = static_cast<TextEntry *>(button);
	int numSample = atoi(te->scratch().c_str());

	if (numSample <= 0)
	{
		return;
	}
	
	_sampleWeight = _currentWeight; // freeze the weight used for these samples
	_hasSampleWeight = true;

	int previousMode = _flex->colIdx(); 
	FlexSample sampler(_flex, _instance);
	sampler.saveSampledStructures(numSample, "sample_structure", "structure_deviation.csv", _sampleWeight);
	_flex->setColIdx(previousMode);
	_flex->submitJobAndRetrieve(_currentWeight); // back to what the slider shows
}


void FlexibilityController::handleDistMatrix()
{

	AskForText *aft = new AskForText(_view, "Enter the column index from _V you want to use:",
	                                 "enter_colIdx", _view, TextEntry::Numeric);
	_view->setModal(aft);
}


void FlexibilityController::handleTargetAtoms()
{
    // if (!_selectFlag)
	// {
	// 	BadChoice *bch = new BadChoice(_view, "Please select hbonds first and then come back for the B-factors");
	// 	_view->setModal(bch);
	// }
	// else
	{
		AskForText *aft = new AskForText(_view, "First target atom (e.g. X-CYS18:SG):",
	                                 "enter_atomA", _view);
		aft->allowCapitals(true);
		_view->setModal(aft);
	}

}

void FlexibilityController::handleTargetA(Button* button)
{ 
	TextEntry *te = static_cast<TextEntry *>(button);
	_targetAtomA = te->scratch();
	trim(_targetAtomA);
	AskForText *aft = new AskForText(_view, "Second target atom (e.g. X-CYS78:SG):",
	                                 "enter_atomB", _view);
	aft->allowCapitals(true);
	_view->setModal(aft);	

}

void FlexibilityController::handleTargetB(Button* button)
{ 
	TextEntry *te = static_cast<TextEntry *>(button);
	_targetAtomB = te->scratch();
	trim(_targetAtomB);
	AtomGroup *atoms = studyInstance()->currentAtoms();
	Atom *targetA = atoms->atomByDesc(_targetAtomA);
	Atom *targetB = atoms->atomByDesc(_targetAtomB);
	if (!targetA || !targetB)
	{
		std::string missing = !targetA ? _targetAtomA : _targetAtomB;
		_view->setModal(new BadChoice(_view, "Atom not found in the structure: " + missing));
		return;
	}
	_flex->setTargetCoordinate(targetA, targetB);
	_flex->setRunLabel("baseline");
    _flex->sensitivityVector();
    _flex->checkGammaSanity();
    _flex->computeInfluenceCoef();
}

void FlexibilityController::handleColumnIdx(Button* button)
{
	TextEntry *te = static_cast<TextEntry *>(button);
	// double w = 10;
	int idx = atof(te->scratch().c_str());
	
	// Python-style negative indexing
    int totalCols = _flex->getVcolumns();
    if (idx < 0)
        idx = totalCols + idx; 

	// bounds check after resolution
    if (idx < 0 || idx >= totalCols)
    {
        std::cerr << "[ERROR] Column index " << idx 
                  << " out of range [0, " << totalCols - 1 << "]" << std::endl;
        return;
    }
	// NEW
	float weight = _hasSampleWeight ? _sampleWeight : _currentWeight;
	std::cout << "[explorer] column " << idx << ", weight " << weight 
			  << (_hasSampleWeight ? " (from last saved samples)" : " (from slider)") 
		      << std::endl;
	// end NEW

	FlexSample sampler(_flex, _instance);
	sampler.computeOneSample(idx, weight);
	std::ostringstream title;
	title << "Column " << idx << ", weight " << weight;
	_view->openAtom2AtomExplorer(title.str());

}

void FlexibilityController::callAddHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs) 
{
	_flex->clearHBonds();
	for (auto &pair : donorAcceptorPairs) 
	{
    	_flex->addHBond(pair);
    }
    _flex->addVnWBond();
    _flex->buildConstraintMap();
    // at the end of callAddHBonds(), after addVnWBond():
	std::cout << "[DEBUG] _hbonds size: " << _flex->getHBonds().size() << std::endl;
	std::cout << "[DEBUG] _VdWBonds size: " << _flex->getVdWBonds().size() << std::endl;

	// const std::vector<AtomBlock> &blocks = _flex->getResources().sequence()->blocks();
	// if (!_flex->getConstraintMap().empty())
	// {
	// 	BondConstraint &hbc = _flex->getConstraintMap().begin->second;
	// 	glm::vec3 axisA = blocks[0].my_positions();
	// 	glm::vec3 axisB = blocks[1].my_positions();
	// 	hbc.hbonds->getDerivative(Distance, axisA, axisB, blocks, 0);
	// }

    FlexibilityCache::instance().store(_instance, _flex);


}

void FlexibilityController::finishedDragging(std::string tag, double x, double y)
{
	// float num = x / 1.;
	// _currentWeight = num;
	// float test_retrival = _flex->submitJobAndRetrieve(num);
	// _first = false;
	_currentWeight = x;
	_hasSampleWeight = false; // the slider is now the altest choice
	_flex->submitJobAndRetrieve(_currentWeight);
	_first = false;
} 









