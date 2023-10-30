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

#include "AxisExplorer.h"
#include "Chain.h"
#include "ColourLegend.h"

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/GuiAtom.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/maths.h>
#include "BondSequence.h"

#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/AtomBlock.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Torsion2Atomic.h>
#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/engine/Tasks.h>

AxisExplorer::AxisExplorer(Scene *prev, Instance *inst, const RTAngles &angles)
: Scene(prev), Display(prev), StructureModification(inst)
{
	_dims = 1;
	_rawAngles = angles;
	setPingPong(true);
	setOwnsAtoms(false);
	_instance->load();
}

AxisExplorer::~AxisExplorer()
{
	_instance->unload();
}

void AxisExplorer::setup()
{
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();
	loadAtoms(grp);
	_fullAtoms = grp;
	
	Display::setup();
	
	prepareResources();
	setupSlider();
	
	submitJob(0.0);
	setupColours();
	setupColourLegend();
	
	VisualPreferences *vp = &_instance->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp, instance());
	
	askForAtomMotions();
}

void AxisExplorer::submitJob(float prop)
{
	submitSingleAxisJob(prop, 0, Flag::AtomVector);

	Result *r = _resources.calculator->acquireResult();
	r->transplantPositions(_displayTargets);

	r->destroy(); delete r;
}

void AxisExplorer::finishedDragging(std::string tag, double x, double y)
{
	submitJob(x);
}

void AxisExplorer::setupSlider()
{
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Extent of motion", _min, _max, _step);
	s->setStart(0.5, 0.);
	s->setCentre(0.5, 0.85);
	_rangeSlider = s;
	addObject(s);
}

void AxisExplorer::adjustTorsions()
{
	Entity *entity = _instance->entity();
	Torsion2Atomic t2a(entity, _cluster, _instance);
	_movement = t2a.convertAnglesSimple(_instance, _rawAngles);
	_movement.attachInstance(_instance);
	_displayTargets = true;

	for (size_t i = 0; i < _movement.size(); i++)
	{
		Atom *atom = _movement.header(i).atom(_instance->currentAtoms());

		if (atom)
		{
			atom->setOtherPosition("target", atom->derivedPosition());
			atom->setOtherPosition("moving", _movement.storage(i));
		}
	}

	CoordManager *manager = _resources.sequences->manager();
	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();
	manager->setAtomFetcher(AtomBlock::prepareMovingTargets(blocks));
}

void AxisExplorer::supplyTorsions(CoordManager *manager)
{
	BondSequenceHandler *sequences = _resources.sequences;
	std::vector<Parameter *> params = sequences->torsionBasis()->parameters();
	RTAngles filtered = _rawAngles;
	filtered.attachInstance(_instance);
	filtered.filter_according_to(params);

	auto grab_torsion = [filtered](const Coord::Get &get, const int &idx) -> float
	{
		return filtered.storage(idx) * get(0);
	};
	
	manager->setTorsionFetcher(grab_torsion);
}

void AxisExplorer::askForAtomMotions()
{
	std::string str = "Would you like to see the atom motions associated\n"
	"with this torsion angle motion?";
	
	AskYesNo *ayn = new AskYesNo(this, str, "adjust", this);
	setModal(ayn);
}

void AxisExplorer::buttonPressed(std::string tag, Button *button)
{
	if (tag == "yes_adjust")
	{
		adjustTorsions();
	}
	else
	{
		std::string str = "Note that this slider linearly interpolates the\n"
		"torsion angle motions. This is rarely valid. Use only\n"
		"as a guide to where the boundaries between rigid domains\n"
		"are, not the motions themselves.";

		BadChoice *bc = new BadChoice(this, str);
		setModal(bc);
	}

	Display::buttonPressed(tag, button);
}

void AxisExplorer::setupColoursForList(RTAngles &angles)
{
	float sum = 0;
	for (size_t i = 0; i < angles.size(); i++)
	{
		const ResidueTorsion &rt = angles.c_rt(i);
		
		if (!rt.torsion().coversMainChain())
		{
			continue;
		}

		const Angular &val = angles.storage(i);
		float sqval = val * val;
		sum += sqval;
	}
	
	sum /= (float)(angles.size());
	
	float stdev = sqrt(sum);
	_maxTorsion = stdev * 4;

	for (size_t i = 0; i < angles.size(); i++)
	{
		const ResidueTorsion &rt = angles.c_rt(i);
		const TorsionRef &tr = rt.torsion();
		
		if (!tr.coversMainChain())
		{
			continue;
		}

		Residue *master = rt.master();
		Residue *local = _instance->equivalentLocal(master);
		if (local == nullptr)
		{
			continue;
		}
		ResidueId id = local->id();
		
		Atom *atom = _fullAtoms->atomByIdName(id, "");
		if (atom == nullptr)
		{
			continue;
		}

		float val = fabs(angles.storage(i));
		val /= _maxTorsion;
		atom->addToColour(val);
	}
}

void AxisExplorer::setupColours()
{
	for (Atom *a : _fullAtoms->atomVector())
	{
		a->setAddedColour(0.f);
	}
	
	setupColoursForList(_rawAngles);
}

void AxisExplorer::setupColourLegend()
{
	ColourLegend *legend = new ColourLegend(Heat, true, this);
	legend->disableButtons();
	legend->setTitle("Torsion deviation (deg)");
	legend->setLimits(0.f, _maxTorsion);
	legend->setCentre(0.9, 0.5);
	addObject(legend);
}

void AxisExplorer::prepareResources()
{
	const int threads = 2;
	_resources.allocateMinimum(threads);

	/* set up per-bond/atom calculation */
	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
	}

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();
	
	supplyTorsions(_resources.sequences->manager());
}

