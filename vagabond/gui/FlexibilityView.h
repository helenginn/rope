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

#ifndef __vagabond__FlexibilityView__
#define __vagabond__FlexibilityView__


#include <vagabond/gui/Display.h>
#include <vagabond/gui/elements/DragResponder.h>
#include <vagabond/gui/HBondMenu.h>
#include <vagabond/core/Flexibility.h>
#include <memory> 


class Slider;
class Residue;
class Instance;
class AtomContent;

// FlexibilityView class - Inherits from Display and DragResponder
class FlexibilityView: public Display, public DragResponder
{
public:
	// Constructor - Takes a pointer to the previous scene, instance, and flexibility object
	FlexibilityView(Scene *prev, Instance *inst, Flexibility *flex);
	// Destructor
	~FlexibilityView();
	void makeMenu();
	void buttonPressed(std::string tag, Button *button);
	// Sets up the view (called during initialization)
	virtual void setup();
	// Sets up the slider element
	void setupSlider();
	void selectMode(HBondMenu *hbmenu, bool flag)
	{
		// HBondMenu *hbmenu = new HBondMenu(this);
		hbmenu->setMode(flag);
	}
	void checkHBondSelection();
	// Submits a flexibility calculation job with a specific weight
	void submitJob(float prop, bool tear = false);
	// Handles finishing drag interaction on the slider (updates flexibility weight)
	virtual void finishedDragging(std::string tag, double x, double y);
	void callAddHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs);	
	int calculateMaximumTorsionSetSize(const std::vector<std::pair<std::string, std::string>> &donorAcceptorPairs);
	void hbondSelected(HBondMenu* hbondMenu, const std::vector<HBondManager::HBondPair> &selectedHBonds);
	void handleHBonds(const std::vector<HBondManager::HBondPair>& pairs);
	void reset();

private: 
	bool _selectFlag = false;
	std::vector<HBondManager::HBondPair> _hBondPairs;
	Flexibility *_flex = nullptr;
	Instance *_instance = nullptr;
	Slider *_rangeSlider = nullptr;
	DisplayUnit *_unit = nullptr;
	AtomMap *_latest = nullptr;
	bool _first = true;

	// Minimum value for the slider
	double _min = -1; 
	// Maximum value for the slider
	double _max = 1; 
	// Step size for the slider
	double _step = 0.001;



};

#endif
