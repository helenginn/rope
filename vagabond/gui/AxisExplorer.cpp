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

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/GuiAtom.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/maths.h>

#include <vagabond/core/MetadataGroup.h>
#include <vagabond/c4x/Cluster.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/ConcertedBasis.h>
#include <vagabond/core/AlignmentTool.h>

AxisExplorer::AxisExplorer(Scene *prev, Molecule *mol,
                           const std::vector<ResidueTorsion> &list,
                           const std::vector<Angular> &values) 
: Scene(prev), Display(prev), StructureModification(mol, 1, 1)
{
//	_pType = BondCalculator::PipelineForceField;
	_dims = 1;
	_list = list;
	_values = values;
	setOwnsAtoms(false);
}

AxisExplorer::~AxisExplorer()
{

}

void AxisExplorer::setup()
{
	_molecule->model()->load();

	AtomGroup *grp = _molecule->currentAtoms();
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
	
	VisualPreferences *vp = &_molecule->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp);
	
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
	
	return;
	
	AtomGroup &ag = *_molecule->model()->currentAtoms();
	Atom *fe = ag.firstAtomWithName("FE");
	glm::vec3 ref = fe->initialPosition();
	
	for (size_t i = 0; i < ag.size(); i++)
	{
		Atom *a = ag[i];
		glm::vec3 pos = a->derivedPosition();
		glm::vec3 init = a->initialPosition();
		double start = glm::length(init - ref);
		double end = glm::length(pos - ref);
		double diff = end - start;
		a->setDerivedBFactor(diff);
	}
	_molecule->model()->write("bfactor.pdb");
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
	props.group = _molecule->currentAtoms();
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
	for (const Angular &val : _values)
	{
		float sqval = val * val;
		sum += sqval;
	}
	
	sum /= (float)(_values.size());
	
	float stdev = sqrt(sum);

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

		Sequence *seq = _molecule->sequence();
		Residue *local = seq->local_residue(master);
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
		val /= stdev * 2;
		atom->addToColour(val);
	}

}
