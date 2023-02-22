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

#include <iostream>
#include <thread>
#include <vagabond/gui/elements/Text.h>

#include "SandboxView.h"
#include "Sequence.h"
#include "AtomGroup.h"
#include "AtomsFromSequence.h"
#include "CyclicView.h"

SandboxView::SandboxView(Scene *prev) : Scene(prev), Display(prev)
{
	CyclicView *cyclic = new CyclicView();
	cyclic->setResponder(this);
	addObject(cyclic);
	_cyclic = cyclic;
	_angles = new Text("angles", Font::Thin, true);
	_angles->resize(0.6);
	_angles->setLeft(0.2, 0.5);
	addObject(_angles);

	_lengths = new Text("lengths", Font::Thin, true);
	_lengths->resize(0.6);
	_lengths->setLeft(0.05, 0.5);
	addObject(_lengths);
	
	Sequence apa("APA");
	AtomGroup *as = AtomsFromSequence(apa).atoms();
	setAtoms(as);
	as->recalculate();
}

SandboxView::~SandboxView()
{
	stop();
}

void SandboxView::setup()
{
	addTitle("Proline sandbox");

	_cyclic->increment();
	doThings();
	
	Display::setup();
}

void SandboxView::buttonPressed(std::string tag, Button *button)
{
	Mouse3D::buttonPressed(tag, button);
}

void SandboxView::stop()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

}

void SandboxView::mouseReleaseEvent(double x, double y,
                                    SDL_MouseButtonEvent button)
{
	Mouse3D::mouseReleaseEvent(x, y, button);

	if (_cyclic && !_moving && _shiftPressed)
	{
		stop();
		updateInfo();
		_worker = new std::thread(&CyclicView::refine, _cyclic);
	}
}

void SandboxView::respond()
{

}

void SandboxView::doThings()
{

}

void SandboxView::updateInfo()
{
	std::string report = "angles\n";
	report += _cyclic->angles();
	_angles->setText(report);

	report = "lengths\n";
	report += _cyclic->lengths();
	_lengths->setText(report);

	float lsc = _cyclic->score();
	setInformation("score: " + std::to_string(lsc));

}
