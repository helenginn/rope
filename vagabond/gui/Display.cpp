#include "Display.h"
#include "GuiAtom.h"
#include "GuiRefls.h"
#include "GuiDensity.h"

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>

#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/ArbitraryMap.h>

#include <SDL2/SDL.h>
#include <iostream>

Display::Display(Scene *prev) : 
Scene(prev),
Mouse3D(prev)
{
	_alwaysOn = true;

}

Display::~Display()
{
	deleteObjects();
	
	if (_owns && _atoms != nullptr)
	{
		delete _atoms;
		_atoms = nullptr;
	}
}

void Display::stop()
{
	if (_guiAtoms != nullptr)
	{
		_guiAtoms->stop();
	}
}

void Display::recalculateAtoms()
{
	AlignmentTool tool(_atoms);
	tool.run();
	_atoms->recalculate();
}

void Display::loadDiffraction(Diffraction *diff)
{
	if (_guiRefls != nullptr)
	{
		removeObject(_guiRefls);
		delete _guiRefls;
	}

	_guiRefls = new GuiRefls();
	_guiRefls->populateFromDiffraction(diff);

	_centre = glm::vec3(0., 0., 0.);
	_translation = -_centre;
	_translation.z -= 10;
	
	updateCamera();

	addObject(_guiRefls);
	wedgeButtons();
	
	_diff = diff;
	_reciprocal = true;

	fftButton();
}

void Display::fftButton()
{
	TextButton *text = new TextButton("Fourier", this);
	text->setRight(0.95, 0.25);
	text->setReturnTag("fft");
	addObject(text);

}

void Display::wedgeButtons()
{
	ImageButton *lemon = new ImageButton("assets/images/lemon.png", this);
	lemon->resize(0.16);
	lemon->setCentre(0.9, 0.1);
	lemon->setReturnTag("half_wedge");
	addObject(lemon);
	_halfWedge = lemon;

	ImageButton *orange = new ImageButton("assets/images/orange.png", this);
	orange->resize(0.16);
	orange->setCentre(0.9, 0.1);
	orange->setReturnTag("full_wedge");
	orange->setDisabled(true);
	_wedge = orange;
	addObject(orange);
}

void Display::mechanicsButton()
{
	ImageButton *mechanics = new ImageButton("assets/images/torsion.png", this);
	mechanics->resize(0.10);
	mechanics->setCentre(0.94, 0.24);
	mechanics->setReturnTag("mechanics");
	_mechanics = mechanics;
	addObject(mechanics);
}

void Display::densityButton()
{
	ImageButton *density = new ImageButton("assets/images/density.png", this);
	density->resize(0.15);
	density->setCentre(0.94, 0.12);
	density->setReturnTag("density");
	_density = density;
	addObject(density);
}

void Display::densityFromDiffraction(Diffraction *diff)
{
	_map = new ArbitraryMap(*diff);
	_map->setup();

	_guiDensity->setReferenceDensity(_map);
}

void Display::loadAtoms(AtomGroup *atoms)
{
	if (_guiAtoms != nullptr)
	{
		removeObject(_guiAtoms);
		delete _guiAtoms;
	}
	
	if (_owns && _atoms != nullptr)
	{
		delete _atoms;
	}
	
	_atoms = atoms;
	_guiAtoms = new GuiAtom();
	_guiAtoms->watchAtoms(_atoms);
	_guiAtoms->startBackgroundWatch();

	_guiDensity = new GuiDensity();
	_guiDensity->setAtoms(_atoms);
	addObject(_guiDensity);

	_centre = _guiAtoms->getCentre();
	_translation = -_centre;
	_translation.z -= 240;

	updateCamera();

	addObject(_guiAtoms);
	
	if (_owns)
	{
		tieButton();
	}
}

void Display::tieButton()
{
	{
		TextButton *button = new TextButton("Tie up with bonds", 
		                                    this);
		button->setReturnTag("recalculate");
		button->resize(0.8);
		button->setCentre(0.5, 0.85);
		addObject(button);
	}
}

void Display::resetDensityMap()
{
	if (_map != nullptr)
	{
		delete _map;
		_map = nullptr;
	}
	
	if (_guiDensity != nullptr)
	{
		removeObject(_guiDensity);
		delete _guiDensity;
		_guiDensity = nullptr;
	}
}

void Display::makeMapFromDiffraction()
{
	resetDensityMap();

	if (_diff)
	{
		_map = new ArbitraryMap(*_diff);
		_map->setup();
		_guiDensity = new GuiDensity();
		_guiDensity->populateFromMap(_map);
		addObject(_guiDensity);
		_guiRefls->setDisabled(true);
	}
	
	_reciprocal = false;
}

void Display::setup()
{
	if (_toLoad != nullptr && _atoms == nullptr)
	{
		loadAtoms(_toLoad);
		_toLoad = nullptr;
	}
}

void Display::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		if (_atoms != nullptr)
		{
			stop();
			_atoms->cancelRefinement();
		}
		
		triggerResponse();
	}

	Scene::buttonPressed(tag, button);

	if (tag == "recalculate")
	{
		recalculateAtoms();
		densityButton();
		mechanicsButton();
		removeObject(button);

		{
			TextButton *replace = new TextButton("Refine model positions", 
			                                    this);
			replace->setReturnTag("refine_positions");
			replace->resize(0.8);
			replace->setCentre(0.5, 0.85);
			addObject(replace);
			removeObject(button);
			delete button;
			return;
		}
	}
	else if (tag == "refine_positions")
	{
		_atoms->refinePositions();
	}
	else if (tag == "fft")
	{
		if (_reciprocal)
		{
			makeMapFromDiffraction();
		}
		else
		{
			_reciprocal = true;
			resetDensityMap();
			_guiRefls->setDisabled(false);
		}
	}
	else if (tag == "full_wedge")
	{
		_halfWedge->setDisabled(false);
		_wedge->setDisabled(true);
		_guiRefls->setSlice(false);
	}
	else if (tag == "half_wedge")
	{
		_halfWedge->setDisabled(true);
		_wedge->setDisabled(false);
		_guiRefls->setSlice(true);
	}
	else if (tag == "density" && _guiDensity != nullptr)
	{
		_guiDensity->recalculate();
	}
	else if (tag == "mechanics")
	{
		_atoms->mechanics();
	}
}

