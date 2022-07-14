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
#include <vagabond/utils/FileReader.h>

#include <vagabond/core/Molecule.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/ConcertedBasis.h>
#include <vagabond/core/AlignmentTool.h>

AxisExplorer::AxisExplorer(Scene *prev, Molecule *mol, 
                           const std::vector<std::string> &list,
                           const std::vector<float> &values) 
: Scene(prev), Display(prev), StructureModification(mol)
{
	_list = list;
	_values = values;
	_dims = 1;
	setOwnsAtoms(false);
}

AxisExplorer::~AxisExplorer()
{

}

void AxisExplorer::setup()
{
	_molecule->model()->load();

	AtomContent *grp = _molecule->model()->currentAtoms();
	AlignmentTool tool(grp);
	tool.run();
	grp->recalculate();

	loadAtoms(grp);
	recalculateAtoms();

	_fullAtoms = grp;
	
	Display::setup();
	
	startCalculator();
	setupSlider();
	
	submitJob(2.0);
	_molecule->model()->write("modified.pdb");
	submitJob(0.0);
	_molecule->model()->write("unmodified.pdb");
	
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
		calc->submitJob(job);
	}

	for (BondCalculator *calc : _calculators)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			return;
		}

		if (r->requests & JobExtractPositions)
		{
			r->transplantLastPosition();
		}
	}
}

void AxisExplorer::fillBasis(ConcertedBasis *cb)
{
	cb->fillFromMoleculeList(_molecule, 0, _list, _values);
	checkMissingBonds(cb);
}

void AxisExplorer::finishedDragging(std::string tag, double x, double y)
{
	submitJob(x * 2);
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

void AxisExplorer::checkMissingBonds(ConcertedBasis *cb)
{
	for (BondTorsion *bt : cb->missingBonds())
	{
		if (bt->coversMainChain())
		{
			_mainMissing++;
		}
		else
		{
			_sideMissing++;
		}
	}
	
	if (_unusedId == nullptr)
	{
		_unusedId = cb->unusedTorsion();
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

