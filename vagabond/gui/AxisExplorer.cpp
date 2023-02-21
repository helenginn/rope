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
#include <vagabond/gui/GuiAtom.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/maths.h>

#include <vagabond/core/MetadataGroup.h>
#include <vagabond/c4x/Cluster.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/AlignmentTool.h>

AxisExplorer::AxisExplorer(Scene *prev, Instance *inst,
                           const std::vector<ResidueTorsion> &list,
                           const std::vector<Angular> &values) 
: Scene(prev), Display(prev), StructureModification(inst, 1, 1)
{
//	_pType = BondCalculator::PipelineForceField;
	_dims = 1;
	_list = list;
	_values = values;
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
	supplyTorsions(_list, _values);
	setupSlider();
	
	submitJob(0.0);
	setupColours();
	setupColourLegend();
	
	VisualPreferences *vp = &_instance->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp, instance());
	
	reportMissing();
}

void AxisExplorer::submitJob(float prop)
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		job.custom.vecs[0].mean[0] = prop;
		job.requests = static_cast<JobType>(JobExtractPositions); 
//		                                    | JobScoreStructure);
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

		if (r->requests & JobExtractPositions)
		{
			r->transplantPositions();
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
}

void AxisExplorer::reportMissing()
{
	if (_mainMissing == 0 && _sideMissing == 0 && _unusedId == nullptr)
	{
		return;
	}
	
	std::string main = i_to_str(_mainMissing);
	std::string side = i_to_str(_sideMissing);

	std::string str;
	if (_mainMissing == 0 && _sideMissing > 0)
	{
		str = "Note: missing torsion angles for\n" + side + 
		" side chain torsions.";
	}
	else if (_sideMissing == 0 && _mainMissing > 0)
	{
		str = "Warning: missing torsion angles for\n" + main + 
		" main chain torsions. This is likely to strongly\n" +
		"disturb the result.";
	}
	else if (_sideMissing > 0 && _mainMissing > 0)
	{
		str = "Warning: missing torsion angles for\n" + main + 
		" main chain and " + side + " side chain torsions.\n" +
		"This is likely to strongly disturb the result.";
	}
	
	if (_unusedId != nullptr)
	{
		if (str.length())
		{
			str += "\nOther torsion angles from reference not used.";
		}
		else
		{
			str = "Torsion angles from second structure\n"\
			"(e.g. " + _unusedId->desc() + ") have no equivalent \n"\
			"in reference. This may disturb the result.";
		}
	}
	
	BadChoice *bc = new BadChoice(this, str);
	setModal(bc);
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

void AxisExplorer::setupColours()
{
	CorrelData cd = empty_CD();
	
	for (Atom *a : _fullAtoms->atomVector())
	{
		a->setAddedColour(0.f);
	}
	
	float sum = 0;
	for (size_t i = 0; i < _list.size(); i++)
	{
		TorsionRef tr = _list[i].torsion;
		
		if (!tr.coversMainChain())
		{
			continue;
		}

		const Angular &val = _values[i];
		float sqval = val * val;
		sum += sqval;
	}
	
	sum /= (float)(_values.size());
	
	float stdev = sqrt(sum);
	_maxTorsion = stdev * 4;

	for (size_t i = 0; i < _list.size(); i++)
	{
		TorsionRef tr = _list[i].torsion;
		
		if (!tr.coversMainChain())
		{
			continue;
		}

		Residue *master = _list[i].residue;
		if (master == nullptr)
		{
			continue;
		}

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

		float val = fabs(_values[i]);
		val /= _maxTorsion;
		atom->addToColour(val);
	}

}

void AxisExplorer::setupColourLegend()
{
	ColourLegend *legend = new ColourLegend(Heat, true, this);
	legend->setTitle("Torsion deviation (deg)");
	legend->setLimits(0.f, _maxTorsion);
	legend->setCentre(0.9, 0.5);
	addObject(legend);

}
