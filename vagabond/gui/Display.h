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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__Display__
#define __vagabond__Display__

#include "DisplayUnit.h"
#include "vagabond/gui/elements/Mouse3D.h"
#include "vagabond/gui/elements/IndexResponseView.h"
#include <vagabond/core/Responder.h>
#include <atomic>

class ImageButton;
class FloatingText;

class Display : 
public Mouse3D, 
public HasResponder<Responder<Display>>,
public IndexResponseView
{
public:
	Display(Scene *prev = nullptr);
	virtual ~Display();
	
	void stopGui();

	virtual void setup();
	
	void addDisplayUnit(DisplayUnit *unit)
	{
		_units.push_back(unit);
	}
	
	void removeDisplayUnit(DisplayUnit *unit)
	{
		auto it = std::find(_units.begin(), _units.end(), unit);
		
		if (it != _units.end())
		{
			delete *it;
			_units.erase(it);
		}
	}
	
	void clearDisplayUnits()
	{
		for (DisplayUnit *unit : _units)
		{
			delete unit;
		}

		_units.clear();
	}

	void recalculateAtoms();
	virtual void tieButton();
	void wedgeButtons();
	void densityButton();
	void mechanicsButton();
	void setPingPong(bool pp)
	{
		_pingPong = pp;
	}
	
	void registerPosition(const glm::vec3 &pos);
	virtual void buttonPressed(std::string tag, Button *button);
	void fftButton();
	
	void supplyFloatingText(FloatingText *text);

	virtual void interactedWithNothing(bool left, bool hover = false);
private:
	void resetDensityMap();

	ImageButton *_halfWedge = nullptr;
	ImageButton *_wedge = nullptr;
	ImageButton *_density = nullptr;
	ImageButton *_mechanics = nullptr;
	
	FloatingText *_text = nullptr;

	bool _pingPong = false;
	std::atomic<bool> _finish{false};
	std::vector<DisplayUnit *> _units;
};

#endif
