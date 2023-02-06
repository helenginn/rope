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

#include "RulesMenu.h"
#include "PathsMenu.h"
#include "AxesMenu.h"
#include "LineSeries.h"
#include "RouteExplorer.h"
#include "PathView.h"
#include "ConfSpaceView.h"
#include "SetupRefinement.h"
#include "Axes.h"
#include "ColourLegend.h"
#include "IconLegend.h"
#include "SerialRefiner.h"
#include "ClusterView.h"
#include "AddModel.h"

#include <vagabond/utils/version.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/core/RopeCluster.h>
#include <vagabond/c4x/ClusterTSNE.h>

#include <vagabond/core/Entity.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/Metadata.h>

ConfSpaceView::ConfSpaceView(Scene *prev, Entity *ent) 
: Scene(prev),
Mouse3D(prev),
IndexResponseView(prev)
{
	_entity = ent;
}

ConfSpaceView::~ConfSpaceView()
{
	deleteObjects();
}

void ConfSpaceView::askToFoldIn(int extra)
{
	std::string str = "Found " + i_to_str(extra) + " unrefined molecules.\n";
	str += "Refine torsion angles to atom positions\nnow?";
	AskYesNo *askyn = new AskYesNo(this, str, "fold_in", this);
	setModal(askyn);
}

void ConfSpaceView::setup()
{
	setMakesSelections();
	IndexResponseView::setup();

	size_t extra = _entity->unrefinedInstanceCount();

	if (extra > 0)
	{
		_extra = extra;
		askToFoldIn(extra);
	}
	else
	{
		refresh();
	}
	
	{
		Text *text = new Text(_entity->name());
		text->setCentre(0.5, 0.80);
		addObject(text);
	}
}

void ConfSpaceView::addGuiElements()
{
	removeRules();
	showClusters();

	applyRules();
	showRulesButton();
	showPathsButton();
	showAxesButton();
//	showtSNE();
}

void ConfSpaceView::showClusters()
{
	ClusterView *view = new ClusterView();
	addIndexResponder(view);
	view->setIndexResponseView(this);
	view->setConfSpaceView(this);
	
	if (_type == ConfTorsions)
	{
		MetadataGroup angles = _entity->makeTorsionDataGroup();
		angles.setWhiteList(_whiteList);
		angles.write(_entity->name() + ".csv");
		angles.normalise();

		RopeCluster *cx = nullptr;
		if (_tsne)
		{
//			cx = new ClusterTSNE<MetadataGroup>(angles);
		}
		else
		{
			cx = new TorsionCluster(angles);
		}

		_cluster = cx;
		cx->cluster();
		view->setCluster(cx);
	}
	
	if (_type == ConfPositional)
	{
		PositionalGroup group = _entity->makePositionalDataGroup();
		group.setWhiteList(_whiteList);

		PositionalCluster *cx = nullptr;
		if (_tsne)
		{
//			cx = new ClusterTSNE<PositionalGroup>(group);
		}
		else
		{
			cx = new PositionalCluster(group);
		}

		_cluster = cx;
		cx->cluster();
		view->setCluster(cx);
	}


	_centre = view->centroid();
	_translation = -_centre;
	_translation.z -= 10;
	updateCamera();

	addObject(view);

	_view = view;
}

void ConfSpaceView::showAxesButton()
{
	ImageButton *b = new ImageButton("assets/images/axes.png", this);
	b->resize(0.1);
	b->setRight(0.97, 0.22);
	b->setReturnTag("axes");

	Text *t = new Text("axes");
	t->resize(0.6);
	t->setRight(0.96, 0.28);
	addObject(b);
	addObject(t);
}

void ConfSpaceView::showPathsButton()
{
	if (_shownPaths)
	{
		return;
	}
	
	int count = Environment::env().pathManager()->objectCount();
	std::cout << "Path count: " << count << std::endl;
	if (count == 0)
	{
		return;
	}

	ImageButton *b = new ImageButton("assets/images/palette.png", this);
	b->resize(0.1);
	b->setRight(0.97, 0.35);
	b->setReturnTag("paths");

	Text *t = new Text("paths");
	t->resize(0.6);
	t->setRight(0.96, 0.41);
	addObject(b);
	addObject(t);
	
	_shownPaths = true;
}

void ConfSpaceView::showRulesButton()
{
	ImageButton *b = new ImageButton("assets/images/palette.png", this);
	b->resize(0.1);
	b->setRight(0.97, 0.09);
	b->setReturnTag("rules");

	Text *t = new Text("rules");
	t->resize(0.6);
	t->setRight(0.96, 0.15);
	addObject(b);
	addObject(t);
}

void ConfSpaceView::showtSNE()
{
	TextButton *t = new TextButton("t-SNE", this);
	t->setReturnTag("tsne");
	t->resize(0.6);
	t->setRight(0.97, 0.8);
	addObject(t);
}

void ConfSpaceView::chooseGroup(Rule *rule, bool inverse)
{
	ObjectGroup *mg = _view->cluster()->objectGroup();
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		Metadata::KeyValues kv = hm->metadata();
		std::string expected = rule->headerValue();
		std::string value;
		
		if (kv.count(rule->header()) > 0)
		{
			value = kv.at(rule->header()).text();
		}
		else if (!rule->ifAssigned())
		{
			continue;
		}

		bool hit = false;

		if (rule->ifAssigned() && value.length())
		{
			hit = true;
		}
		else if (!rule->ifAssigned())
		{
			hit = (value == expected);
		}

		if (inverse)
		{
			hit = !hit;
		}

		if (hit)
		{
			whiteList.push_back(hm);
		}
	}
	
	std::cout << "Collected " << whiteList.size() << " molecules." << std::endl;
	
	ConfSpaceView *view = new ConfSpaceView(this, _entity);
	view->setMode(_type);
	view->setWhiteList(whiteList);
	view->show();
}

void ConfSpaceView::executeSubset(float min, float max)
{
	ObjectGroup *mg = _view->cluster()->objectGroup();
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		Metadata::KeyValues kv = hm->metadata();
		
		if (kv.count(_colourRule->header()) > 0)
		{
			float num = kv.at(_colourRule->header()).number();
			
			if (num >= min && num <= max)
			{
				whiteList.push_back(hm);
			}
		}
	}
	
	std::cout << "Collected " << whiteList.size() << " molecules." << std::endl;
	
	ConfSpaceView *view = new ConfSpaceView(this, _entity);
	view->setMode(_type);
	view->setWhiteList(whiteList);
	view->show();
}

void ConfSpaceView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "show_origin")
	{
		delete _origin; _origin = nullptr;
		Axes *axes = new Axes(_cluster);
		addObject(axes);
		_origin = axes;
	}
	if (tag == "hide_origin")
	{
		removeObject(_origin);
		delete _origin; _origin = nullptr;
	}
	if (tag == "choose_reorient_molecule")
	{
		setInformation("choose molecule to reorient axis direction...");
		_status = Reorienting;
	}
	if (tag == "choose_subset")
	{
		std::string str = "Focus on " + _colourRule->header() + " subset:";
		ChooseRange *cr = new ChooseRange(this, str, "execute_subset", 
		                                  this, true);
		cr->setRange(_colourRule->min(), _colourRule->max(), 100);
		setModal(cr);
	}

	if (tag == "execute_subset") // from colour
	{
		ChooseRange *cr = static_cast<ChooseRange *>(button->returnObject());
		float min = cr->min();
		float max = cr->max();
		executeSubset(min, max);
		std::cout << "chosen " << min << " to " << max << std::endl;
	}

	if (tag == "choose_group" || tag == "choose_inverse") // from icon
	{
		Rule *rule = static_cast<Rule *>(button->returnObject());
		chooseGroup(rule, (tag == "choose_inverse"));
	}

	if (tag == "separate_average")
	{
		Rule *rule = static_cast<Rule *>(button->returnObject());
		std::vector<HasMetadata *> members = _view->membersForRule(rule);
		ObjectGroup *mdg = _cluster->objectGroup();
		mdg->setSeparateAverage(members);
		_view->cluster()->cluster();
		refresh();
	}

	if (tag == "match_colour")
	{
		std::cout << _axes << " " << _colourRule << std::endl;
		std::string key = _colourRule->header();
		_axes->backgroundPrioritise(key);
	}

	if (tag == "align_axes")
	{
		AskYesNo *askyn = new AskYesNo(this, "Prioritise PCA axes to best\n"
		                               "represent the colour legend?", 
		                               "align_axes", this);
		setModal(askyn);
	}
	if (tag == "yes_align_axes")
	{
		std::string key = _colourRule->header();
		_view->prioritiseMetadata(key);
		applyRules();
	}
	
	if (tag == "tsne")
	{
		ConfSpaceView *view = new ConfSpaceView(this, _entity);
		view->setMode(_type);
		view->setWhiteList(_whiteList);
		view->setTSNE(true);
		view->show();
	}

	if (tag == "yes_fold_in")
	{
		// refine extra molecules
		SerialRefiner *refiner = new SerialRefiner(this, _entity);
		refiner->setRefineList(_entity->unrefinedModels());
		refiner->setJobType(rope::Refine);
		refiner->show();
	}
	if (tag == "no_fold_in")
	{
		AskYesNo *askyn = new AskYesNo(this, "Use torsion angles without\n"
		                               "refinement? (quick and dangerous)", 
		                               "use_unrefined", this);
		setModal(askyn);
	}
	if (tag == "yes_use_unrefined")
	{
		// refine extra molecules
		SerialRefiner *refiner = new SerialRefiner(this, _entity);
		refiner->setRefineList(_entity->unrefinedModels());
		refiner->setJobType(rope::SkipRefine);
		refiner->show();
	}
	if (tag == "no_use_unrefined")
	{
		// show conf space
		refresh();
	}
	
	if (tag == "path_from")
	{
		Polymer *m = static_cast<Polymer	*>(button->returnObject());
		_from = m;
	}
	
	if (tag == "view_model")
	{
		Polymer *m = static_cast<Polymer	*>(button->returnObject());
		
		if (m)
		{
			AddModel *am = new AddModel(this, m->model());
			am->setDeleteAllowed(false);
			am->show();
		}
	}
	
	if (tag == "refinement_setup")
	{
		Polymer *m = static_cast<Polymer	*>(button->returnObject());
		createReference(m);
		SetupRefinement *sr = new SetupRefinement(this, *(m->model()));
		sr->setAxes(_axes);
		sr->show();
	}
	
	if (tag == "set_as_reference")
	{
		Polymer *m = static_cast<Polymer	*>(button->returnObject());
		createReference(m);
	}
	
	if (tag == "paths")
	{
		PathsMenu *menu = new PathsMenu(this);
		menu->setEntityId(_entity->name());
		menu->show();
	}
	
	if (tag == "rules")
	{
		RulesMenu *menu = new RulesMenu(this);
		menu->setEntityId(_entity->name());
		menu->setData(_view->cluster()->objectGroup());
		menu->show();
	}
	
	if (tag == "axes")
	{
		if (_cluster->canMapVectors())
		{
			AxesMenu *menu = new AxesMenu(this);
			menu->setEntityId(_entity->name());
			menu->setCluster(_cluster);
			menu->show();
		}
	}

	Scene::buttonPressed(tag, button);
}

void ConfSpaceView::refresh()
{
	try
	{
		if (_view == nullptr)
		{
			addGuiElements();
		}

		_view->refresh();
		applyRules();
		showPathsButton();
	}
	catch (std::runtime_error &err)
	{
		BadChoice *bc = new BadChoice(_previous, err.what());
		_previous->setModal(bc);
		back();
		return;
	}
}

void ConfSpaceView::applyRule(const Rule &r)
{
	if (_view == nullptr)
	{
		return;
	}

	if (r.type() == Rule::LineSeries)
	{
		LineSeries *ls = new LineSeries(_view, r);
		addObject(ls);
		_temps.push_back(ls);
	}
	else
	{
		_view->applyRule(r);
	}
	
	if (r.type() == Rule::VaryColour)
	{
		ColourLegend *legend = new ColourLegend(r.scheme(), false, this);
		legend->setCentre(0.5, 0.1);
		legend->setTitle(r.header());
		legend->setLimits(r.min(), r.max());
		addObject(legend);
		_temps.push_back(legend);
		_colourRule = &r;
	}
}

void ConfSpaceView::removeRules()
{
	_colourRule = nullptr;
	
	if (_view)
	{
		_view->clearRules();
	}

	for (size_t i = 0; i < _temps.size(); i++)
	{
		removeObject(_temps[i]);
		delete _temps[i];
	}

	_temps.clear();
}

void ConfSpaceView::applyRules()
{
	_view->reset();
	removeRules();

	IconLegend *il = new IconLegend(this);

	const Ruler &ruler = Environment::metadata()->ruler();

	for (const Rule &r : ruler.rules())
	{
		applyRule(r);
		il->addRule(&r);
	}
	
	il->makePoints();
	il->setCentre(0.8, 0.5);
	addObject(il);
	_temps.push_back(il);
}

void ConfSpaceView::prepareModelMenu(HasMetadata *hm)
{
	Menu *m = new Menu(this);
	m->setReturnObject(hm);
	m->addOption("view model", "view_model");
	m->addOption("set as reference", "set_as_reference");
	m->addOption("refinement setup", "refinement_setup");
	double x = _lastX / (double)_w; double y = _lastY / (double)_h;
	m->setup(x, y);

	setModal(m);
}

void ConfSpaceView::createReference(Polymer *m)
{
	if (_type != ConfTorsions)
	{
		return;
	}

	Axes *old = _axes;
	if (_axes != nullptr)
	{
		removeObject(_axes);
		removeResponder(_axes);
	}
	
	TorsionCluster *tc = static_cast<TorsionCluster *>(_cluster);
	
	_axes = new Axes(tc, m);
	_axes->setScene(this);
	_axes->setIndexResponseView(this);
	
	if (old)
	{
		_axes->takeOldAxes(old);
		delete old;
	}

	addObject(_axes);
	addIndexResponder(_axes);
}


void ConfSpaceView::reorientToPolymer(Polymer *pol)
{
	if (_axes == nullptr)
	{
		return;
	}
	
	setInformation("");
	_status = Nothing;
	
	_axes->reorient(-1, pol);
}

void ConfSpaceView::prepareEmptySpaceMenu()
{
	Menu *m = new Menu(this);
	
	if (_origin == nullptr)
	{
		m->addOption("show origin", "show_origin");
	}
	else
	{
		m->addOption("hide origin", "hide_origin");
	}

	double x = _lastX / (double)_w; double y = _lastY / (double)_h;
	m->setup(x, y);

	setModal(m);

}

void ConfSpaceView::interactedWithNothing(bool left)
{
	if (!left && !_moving)
	{
		prepareEmptySpaceMenu();
	}

}

void ConfSpaceView::sendSelection(float t, float l, float b, float r)
{

}

void ConfSpaceView::sendObject(std::string tag, void *object)
{

}

void ConfSpaceView::doThings()
{
	if (_routeExplorer != nullptr)
	{
		std::cout << "Showing" << std::endl;
		_routeExplorer->show();
		_routeExplorer = nullptr;
	}
}
