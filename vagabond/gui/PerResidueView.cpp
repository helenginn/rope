// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "PerResidueView.h"
#include "SequenceSlider.h"

#include <vagabond/core/Entity.h>
#include <vagabond/core/Residue.h>

PerResidueView::PerResidueView(Scene *prev, Entity *entity)
: Scene(prev), _entity(entity)
{

}

void PerResidueView::setup()
{
	addTitle("Per-residue Occupancy");

	SequenceSlider *slider = new SequenceSlider(_entity->sequence());
	slider->setBounds(0.1, 0.35, 0.9, 0.55, 0.62);

	auto onResidue = [this](Residue *r)
	{
		_selectedResidue = r;
	};

	slider->setReturnJob(onResidue);
	slider->setup();

	addObject(slider);
	_slider = slider;
}
