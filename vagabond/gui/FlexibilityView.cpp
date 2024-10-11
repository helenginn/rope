#include "FlexibilityView.h"

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>

#include <vagabond/core/Instance.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/Flexibility.h>



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
	_instance->unload();
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
		m->setup(c.x, c.y);
		setModal(m);
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
	Display::buttonPressed(tag, button);
}


void FlexibilityView::setup()
{
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();
	DisplayUnit *unit = new DisplayUnit(this);
	// unit->loadAtoms(grp, _instance->entity());
	unit->loadAtoms(grp, _instance->entity());
	unit->displayAtoms();
	addDisplayUnit(unit);

	Display::setup();
	_flex->prepareResources();
	setupSlider();
	_flex->submitJobAndRetrieve(0.0);
	// _flex->submitJobAndRetrieve(0.0);
	makeMenu();
	// Create a vector of donor-acceptor pairs
    std::vector<std::pair<std::string, std::string>> donorAcceptorPairs = 
    {
        {"A-ILE3:N", "A-ASP32:O"},
        {"A-TYR22:N", "A-SER18:O"}
    };
    _flex->addMultipleHBonds(donorAcceptorPairs);
    // callAddHBonds(donorAcceptorPairs);
	_flex->printHBonds();
}

void FlexibilityView::callAddHBonds(const std::vector<std::pair<std::string, std::string>> &donorAcceptorPairs) 
{
	for (const auto& pair : donorAcceptorPairs) 
	{
		std::cout << "Calling callAddHbonds..." << std::endl;
    	_flex->addHBond(pair.first, pair.second);
    }
}

void FlexibilityView::setupSlider()
{
	removeObject(_rangeSlider);
	delete _rangeSlider;
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Flexibility amplifier", _min, _max, _step);
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








