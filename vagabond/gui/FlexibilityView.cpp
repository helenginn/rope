#include "FlexibilityView.h"

#include <vagabond/gui/elements/Slider.h>

#include <vagabond/core/Instance.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/Flexibility.h>



FlexibilityView::FlexibilityView(Scene *prev, Instance *inst, Flexibility *flex)
: Scene(prev), Display(prev)
{
	_flex = flex;
	_instance = inst;
	// setup();
}

FlexibilityView::~FlexibilityView()
{
	stopGui();
	_instance->unload();
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
	_flex->submitJobAndRetrieve(0.0);
	_flex->submitJobAndRetrieve(0.0);


	// Create a vector of donor-acceptor pairs
    std::vector<std::pair<std::string, std::string>> donorAcceptorPairs = 
    {
        {"A-ILE3:N", "A-ASP32:O"},
        {"A-TYR22:N", "A-SER18:O"}
    };
    callAddHBonds(donorAcceptorPairs);
	_flex->printHBonds();
	setupSlider();
	// askForAtomFlexibility();
}

void FlexibilityView::callAddHBonds(const std::vector<std::pair<std::string, std::string>> &donorAcceptorPairs) 
{
	for (const auto& pair : donorAcceptorPairs) 
	{
		std::cout << "Calling callAddHbonds..." << std::endl;
    	_flex->addHBond(pair.first, pair.second);
    	// before turning vector into a set I want to put together all the the vectors into one
    }

    // Ask tp build the Jacobian matrix

    // caclulate number of columns
    int colJac = donorAcceptorPairs.size();

    // calculate number of rows
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








