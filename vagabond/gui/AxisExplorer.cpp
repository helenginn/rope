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

#include <vagabond/core/MetadataGroup.h>
#include <vagabond/c4x/Cluster.h>
#include <vagabond/core/BondSequence.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/Torsion2Atomic.h>

AxisExplorer::AxisExplorer(Scene *prev, Instance *inst, const RTAngles &angles)
: Scene(prev), Display(prev), StructureModification(inst, 1, 1)
{
	_dims = 1;
	_torsionLists.push_back(angles);
	setPingPong(true);
	setOwnsAtoms(false);
}

AxisExplorer::~AxisExplorer()
{
	_instance->unload();
}

void AxisExplorer::setup()
{
	_instance->load();
	AtomGroup *grp = _instance->currentAtoms();
	AlignmentTool tool(grp);
	tool.run();
	grp->recalculate();

	loadAtoms(grp);
	recalculateAtoms();

	_fullAtoms = grp;
	
	Display::setup();
	
	startCalculator();
	supplyTorsionLists();
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
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.pos_sampler = this;
		job.custom.allocate_vectors(1, _dims, _num);
		job.custom.vecs[0].mean[0] = prop;
		job.requests = static_cast<JobType>(JobPositionVector); 
		calc->submitJob(job);
	}

	double sum = 0;

	for (BondCalculator *calc : _calculators)
	{

		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			return;
		}

		if (r->requests & JobPositionVector)
		{
			r->transplantPositions(_displayTargets);
			sum += r->score;
		}
	}
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
	std::cout << "Torsion to atomic: " << std::endl;
	Entity *entity = _instance->entity();
	Torsion2Atomic t2a(entity, _cluster);
	_movement = t2a.convertAngles(_torsionLists[0]);
	
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
		_displayTargets = true;
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

void AxisExplorer::customModifications(BondCalculator *calc, bool has_mol)
{
	if (!has_mol)
	{
		return;
	}

	calc->setPipelineType(_pType);
	FFProperties props;
	props.group = _instance->currentAtoms();
	props.t = FFProperties::CAlphaSeparation;
	calc->setForceFieldProperties(props);
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
		
		Atom *atom = _fullAtoms->atomByIdName(id, "CA");
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
	
	if (_torsionLists.size())
	{
		setupColoursForList(_torsionLists[0]);
	}
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

int findAtom(RAMovement &movement, Atom *search, Instance *instance)
{
	for (size_t i = 0; i < movement.size(); i++)
	{
		if (movement.header(i).fitsAtom(search, instance))
		{
			return i;
		}
	}
	
	return -1;
}

bool AxisExplorer::prewarnAtoms(BondSequence *seq, const Coord::Get &get,
                                Vec3s &positions)
{
	if (_movement.size() == 0)
	{
		return false;
	}

	const std::vector<AtomBlock> &blocks = seq->blocks();
	positions.resize(blocks.size());

	for (int i = 0; i < blocks.size(); i++)
	{
		Atom *search = blocks[i].atom;
		if (!search)
		{
			continue;
		}
		
		glm::vec3 pos = search->initialPosition();
		int movement_idx = _mapping(seq, i);
		
		if (movement_idx < 0)
		{
			int idx = findAtom(_movement, search, _instance);
			_mapping.setSeqBlockIdx(seq, i, idx);
			
			if (idx < 0)
			{
				idx = _movement.size();
			}

			movement_idx = idx;
		}
		else if (movement_idx >= _movement.size())
		{
			continue;
		}

		glm::vec3 extra = _movement.storage(movement_idx) * get(0);
		positions[i] = pos + extra;
	}

	return true;
}

