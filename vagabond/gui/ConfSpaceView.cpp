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
#include "RopeSpaceItem.h"
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

using namespace rope;
std::map<Entity *, RopeSpaceItem *> ConfSpaceView::_savedSpaces;

ConfSpaceView::ConfSpaceView(Scene *prev, Entity *ent) 
: Scene(prev),
Mouse3D(prev),
IndexResponseView(prev)
{
	_entity = ent;
}

bool ConfSpaceView::makeFirstCluster()
{
	if (_ropeSpace != nullptr)
	{
		return false;
	}
	
	if (_savedSpaces.count(_entity) > 0)
	{
		_ropeSpace = _savedSpaces[_entity];
		_ropeSpace->attachExisting(this);
	}
	else
	{
		_ropeSpace = new RopeSpaceItem(_entity);
		_ropeSpace->setMode(_type);
		_ropeSpace->makeView(this);
		_savedSpaces[_entity] = _ropeSpace;
	}

	_selected = _ropeSpace;

	return true;
}

ConfSpaceView::~ConfSpaceView()
{
	removeObject(_axes);
	removeObject(_view);
	deleteObjects();

	Item::resolveDeletions();
}

void ConfSpaceView::askToFoldIn(int extra)
{
	size_t prolines = _entity->unrefinedProlineCount();

	std::string str = "Found " + i_to_str(extra) + " unrefined molecules";
	if (prolines == 0)
	{
		str += ".\n";
	}
	else
	{
		str += "\nincluding " + std::to_string(prolines) + " with naughty prolines.\n";
	}
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

void ConfSpaceView::displayTree()
{
	if (_ropeTree)
	{
		removeObject(_ropeTree);
		delete _ropeTree;
		_ropeTree = nullptr;
	}

	if (_ropeSpace->itemCount() == 0)
	{
		return;
	}

	_ropeTree = new LineGroup(_ropeSpace, this);
	_ropeTree->setLeft(0.02, 0.2);
	addObject(_ropeTree);
}

void ConfSpaceView::proofRopeSpace()
{
	if (_selected != nullptr)
	{
		while (_selected->isDeleted())
		{
			_selected = static_cast<RopeSpaceItem *>(_selected->parent());
		}
	}
	
	if (_selected == nullptr)
	{
		_selected = _ropeSpace;
	}
}

void ConfSpaceView::switchView()
{
	removeObject(_view);
	_view = nullptr;
	
	removeObject(_axes);
	_axes = nullptr;
	
	clearResponders();

	bool first = makeFirstCluster();
	proofRopeSpace();
	std::cout << _selected->displayName() << std::endl;
	ClusterView *view = _selected->view();
	glm::vec3 c = view->centroid();
	float distance = first ? 10 : 0;
	shiftToCentre(c, distance);
	_cluster = view->cluster();

	_axes = _selected->axes();
	_view = view;

	addObject(_axes);
	addObject(view);

	_selected->attachExisting(this);
}

void ConfSpaceView::addGuiElements()
{
	showCurrentCluster();
	showRulesButton();
	showPathsButton();
	showAxesButton();
}

void ConfSpaceView::showCurrentCluster()
{
	removeRules();
	switchView();
	applyRules();
	displayTree();
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
	RopeSpaceItem *subset = _selected->branchFromRule(rule, inverse);
	_selected = subset;
	showCurrentCluster();
}

void ConfSpaceView::executeSubset(float min, float max)
{
	RopeSpaceItem *subset = _selected->branchFromRuleRange(_colourRule, 
	                                                       min, max);
	_selected = subset;
	showCurrentCluster();
}

void ConfSpaceView::grabSelection(bool inverse)
{
	RopeSpaceItem *subset = _selected->makeGroupFromSelected(inverse);
	_selected = subset;
	showCurrentCluster();
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
		_selected->deleteAxes();
	}
	if (tag == "selection_group")
	{
		grabSelection(false);
		return;
	}
	if (tag == "selection_inverse")
	{
		grabSelection(true);
		return;
	}
	if (tag == "selection_clear")
	{
		_view->deselect();
		applyRules();
		return;
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
	
	if (tag == "average_clear")
	{
		ObjectGroup *mdg = _cluster->objectGroup();
		mdg->clearAverages();
		_view->cluster()->cluster();
		refresh();
	}

	if (tag == "selection_separate_average")
	{
		std::vector<HasMetadata *> members = _view->selectedMembers();
		ObjectGroup *mdg = _cluster->objectGroup();
		mdg->setSeparateAverage(members);
		_view->cluster()->cluster();
		refresh();
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
	
	/*
	if (tag == "tsne")
	{
		ConfSpaceView *view = new ConfSpaceView(this, _entity);
		view->setMode(_type);
		view->setWhiteList(_whiteList);
		view->setTSNE(true);
		view->show();
	}
	*/

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
		Instance *i = static_cast<Instance *>(button->returnObject());
		createReference(i);
		SetupRefinement *sr = new SetupRefinement(this, *(i->model()));
		sr->setAxes(_axes);
		sr->show();
	}
	
	if (tag == "set_as_reference")
	{
		Instance *i = static_cast<Instance *>(button->returnObject());
		createReference(i);
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
	
	std::string select = Button::tagEnd(tag, "select_");
	
	if (select.length())
	{
		Item *item = Item::itemForTag(select);
		RopeSpaceItem *rsi = static_cast<RopeSpaceItem *>(item);
		_selected = rsi;
		showCurrentCluster();
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
		addTempObject(ls);
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
		addTempObject(legend);
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

	deleteTemps();
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
	addTempObject(il);
	
	_view->applySelected();
}

void ConfSpaceView::prepareModelMenu(HasMetadata *hm)
{
	Menu *m = new Menu(this);
	m->setReturnObject(hm);
	m->addOption("view details", "view_model");
	m->addOption("set as reference", "set_as_reference");
#ifdef VERSION_REFINEMENT
	m->addOption("refinement setup", "refinement_setup");
#endif
	float x; float y;
	getFractionalPos(x, y);
	m->setup(x, y);

	setModal(m);
}

void ConfSpaceView::createReference(Instance *i)
{
	_axes = _selected->createReference(i);
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
	
	if (_origin != nullptr || _axes != nullptr)
	{
		m->addOption("hide origin", "hide_origin");
	}
	
	if (_selected->selectedCount() > 0)
	{
		m->addOption("select group", "selection_group");
		m->addOption("select inverse", "selection_inverse");
		m->addOption("separate average", "selection_separate_average");
	}

	if (_selected->separateAverageCount() > 0)
	{
		m->addOption("clear averages", "average_clear");
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

	if (_shiftPressed && left && !_moving)
	{
		_view->deselect();
		applyRules();
	}
}

void ConfSpaceView::sendSelection(float t, float l, float b, float r,
                                  bool inverse)
{
	convertToGLCoords(&l, &t);
	convertToGLCoords(&r, &b);
	convertGLToHD(l, t);
	convertGLToHD(r, b);

	std::set<int> results = objectsInBox(t, l, b, r);
	
	selectIndices(results, inverse);

	applyRules();
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
