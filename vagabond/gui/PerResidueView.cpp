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
#include <vagabond/core/IndexedSequence.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/RotamerOccupancy.h>

#include <set>

PerResidueView::PerResidueView(Scene *prev, Entity *entity,
                                std::shared_ptr<RotamerOccupancy> rota)
: Scene(prev), _entity(entity), _rota(rota)
{

}

void PerResidueView::setup()
{
	addTitle("Per-residue Occupancy");

	IndexedSequence *sequence = _entity->sequence();

	SequenceSlider *slider = new SequenceSlider(sequence);
	slider->setBounds(0.1, 0.65, 0.9, 0.85, 0.9);

	auto onResidue = [this](Residue *r)
	{
		_selectedResidue = r;
	};

	slider->setReturnJob(onResidue);
	slider->setup();

	_rota->ensureModelsLoaded();

	std::set<Residue *> enabled;
	for (size_t i = 0; i < sequence->entryCount(); i++)
	{
		if (!sequence->hasResidue(0, i))
		{
			continue;
		}

		Residue *r = sequence->residue(0, i);
		if (_rota->hasAltConformers(r))
		{
			enabled.insert(r);
		}
	}

	_rota->unloadModels();

	slider->setEnabledResidues(enabled);

	addObject(slider);
	_slider = slider;
}
