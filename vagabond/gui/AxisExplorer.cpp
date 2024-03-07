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

#include <vagabond/c4x/ClusterSVD.h>

#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/GuiAtom.h>
#include <vagabond/gui/TableView.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/maths.h>

#include <vagabond/core/engine/ElectricFielder.h>
#include <vagabond/core/engine/Task.h>
#include <vagabond/core/grids/AtomMap.h>
#include <vagabond/core/grids/ArbitraryMap.h>
#include <vagabond/core/TabulatedData.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/AtomBlock.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Torsion2Atomic.h>
#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/files/MtzFile.h>
#include <vagabond/core/engine/Tasks.h>

#include "AxisExplorer.h"
#include "Chain.h"
#include "ColourLegend.h"
#include "BondSequence.h"

AxisExplorer::AxisExplorer(Scene *prev, Instance *inst, const RTAngles &angles)
: Scene(prev), Display(prev)
{
	setInstance(inst);
	_dims = 1;
	_rawAngles = angles;
	setPingPong(true);
	_instance->load();
}

AxisExplorer::~AxisExplorer()
{
	_instance->unload();
}

void AxisExplorer::makeMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setReturnTag("menu");
	text->setRight(0.95, 0.1);
	addObject(text);

}

void AxisExplorer::setup()
{
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();
	_fullAtoms = grp;
	
	DisplayUnit *unit = new DisplayUnit(this);
	unit->loadAtoms(grp, _instance->entity());
	unit->displayAtoms();
	addDisplayUnit(unit);
	_unit = unit;
	
	Display::setup();
	
	prepareResources();
	setupSlider();
	
	submitJobAndRetrieve(0.0);
	setupColours();
	setupColourLegend();
	makeMenu();
	
	askForAtomMotions();
}

void AxisExplorer::submitJobAndRetrieve(float prop, bool tear)
{
	submitJob(prop, tear);

	Result *r = _resources.calculator->acquireResult();
	r->transplantPositions(_displayTargets);
	
	if (_latest)
	{
		delete _latest;
	}
	
	_unit->refreshDensity(r->map, true);
	_latest = r->map;
	r->map = nullptr;

	r->destroy(); 

}

void AxisExplorer::submitJob(float prop, bool tear)
{
	Task<Result, void *> *submit = nullptr;
	CalcTask *calc = nullptr;
	Task<BondSequence *, void *> *let_sequence_go = nullptr;
	
	submit = submitSingleAxisJob(prop, 0, Flag::AtomVector, &calc, 
	                             &let_sequence_go);
	
	if (_electric && _unit)
	{
		_resources.electricField->positionsToElectricField(calc, submit,
		                                                   let_sequence_go);
	}
}

void AxisExplorer::finishedDragging(std::string tag, double x, double y)
{
	submitJobAndRetrieve(x, _first);
	_first = false;
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
	Torsion2Atomic t2a(entity, _cluster, _tData, _instance);
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
	if (tag == "menu")
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "options");
		m->addOption("Table for main chain", "main_chain");
		m->addOption("Save state as PDB", "save_state");
		if (_electric)
		{
			m->addOption("Save electric field", "electric_field");
		}
		m->setup(c.x, c.y);
		setModal(m);
	}
	else if (tag == "options_save_state")
	{
		AskForText *aft = new AskForText(this, "PDB file name to save to:",
		                                 "export_pdb", this);
		setModal(aft);
	}
	else if (tag == "options_main_chain")
	{
		TableView *tv = new TableView(this, _data, 
		                              "Main chain torsion variation");
		tv->show();

	}
	else if (tag == "options_electric_field")
	{
		if (_latest)
		{
			Diffraction *diff = _instance->model()->getDiffraction();
			if (!diff)
			{
				return;
			}

			ArbitraryMap *clear = new ArbitraryMap(*diff);
			clear->setupFromDiffraction();
			clear->clear();
			ArbitraryMap *map = (*_latest)();
			*clear += *map;
			MtzFile file;
			file.setMap(clear);
			file.write_to_file("efield.mtz", 2);
			delete map;
			delete clear;
		}

	}
	else if (tag == "yes_adjust")
	{
		adjustTorsions();
	}
	else if (tag == "export_pdb")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string filename = te->scratch();

		std::string path = getPath(filename);
		std::string file = getFilename(filename);
		check_path_and_make(path);

		_instance->currentAtoms()->writeToFile(filename);
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
	std::map<ResidueId, float> list;

	for (size_t i = 0; i < angles.size(); i++)
	{
		ResidueTorsion &rt = angles.rt(i);
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

		float val = angles.storage(i);
		float sqval = val * val;
		list[id] += sqval;
	}
	
	float sqdev = 0;
	for (auto it = list.begin(); it != list.end(); it++)
	{
		it->second = sqrt(it->second);
		sqdev += it->second * it->second;
	}

	sqdev /= (float)(list.size());
	float stdev = sqrt(sqdev);
	_maxTorsion = stdev * 3;
	
	if (_data)
	{
		delete _data;
	}
	
	_data = new TabulatedData({{"Residue number", TabulatedData::Number},
		                     {"Angle variation (deg)", TabulatedData::Number}});

	for (auto it = list.begin(); it != list.end(); it++)
	{
		const ResidueId &id = it->first;
		Atom *atom = _fullAtoms->atomByIdName(id, "");
		float variation = it->second / (_maxTorsion);

		atom->setAddedColour(variation);
		_data->addEntry({{"Residue number", it->first.str()},
			             {"Angle variation (deg)", 
		                   std::to_string(variation)}});
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
	
	_resources.electricField = new ElectricFielder(1);
	_resources.electricField->supplyAtomGroup(group->atomVector());
	_resources.electricField->setup();
	
	supplyTorsions(_resources.sequences->manager());
}

