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
#include "LineSeries.h"
#include "ConfSpaceView.h"
#include "ColourLegend.h"
#include "IconLegend.h"
#include "SerialRefiner.h"
#include "ClusterView.h"

#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/c4x/ClusterSVD.h>

#include <vagabond/core/Entity.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/Metadata.h>

ConfSpaceView::ConfSpaceView(Scene *prev, Entity *ent) : Mouse3D(prev)
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
	size_t extra = _entity->checkForUnrefinedMolecules();

	if (extra > 0)
	{
		_extra = extra;
		askToFoldIn(extra);
	}
	else
	{
		showClusters();
		applyRules();
		showRulesButton();
	}
	
	{
		Text *text = new Text(_entity->name());
		text->setCentre(0.5, 0.85);
		addObject(text);
	}
}

void ConfSpaceView::showClusters()
{
	MetadataGroup angles = _entity->makeTorsionDataGroup();
	angles.write(_entity->name() + ".csv");
	angles.normalise();
	
	ClusterSVD<MetadataGroup> *cx = new ClusterSVD<MetadataGroup>(angles);
	cx->setType(PCA::Correlation);
	cx->cluster();
	
	ClusterView *view = nullptr;
	view = new ClusterView();
	view->setCluster(cx);

	_centre = view->centroid();
	_translation = -_centre;
	_translation.z -= 10;
	updateCamera();

	addObject(view);

	_view = view;
}

void ConfSpaceView::showRulesButton()
{
	ImageButton *b = new ImageButton("assets/images/palette.png", this);
	b->resize(0.1);
	b->setRight(0.95, 0.09);
	b->setReturnTag("rules");

	Text *t = new Text("settings");
	t->resize(0.6);
	t->setRight(0.95, 0.15);

	addObject(b);
	addObject(t);

}

void ConfSpaceView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "yes_fold_in")
	{
		// refine extra molecules
		SerialRefiner *refiner = new SerialRefiner(this, _entity);
		refiner->setExtra(_extra);
		refiner->show();

	}
	if (tag == "no_fold_in")
	{
		// show conf space
		showClusters();
		applyRules();
		showRulesButton();
	}
	
	if (tag == "rules")
	{
		RulesMenu *menu = new RulesMenu(this);
		menu->setEntityId(_entity->name());
		menu->setData(_view->cluster()->dataGroup());
		menu->show();
	}

	Scene::buttonPressed(tag, button);
}

void ConfSpaceView::refresh()
{
	if (_view == nullptr)
	{
		showClusters();
		showRulesButton();
	}

	applyRules();
}

void ConfSpaceView::applyRule(const Rule &r)
{
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
		ColourLegend *legend = new ColourLegend(r.scheme());
		legend->setCentre(0.5, 0.1);
		legend->setTitle(r.header());
		legend->setLimits(r.min(), r.max());
		addObject(legend);
		_temps.push_back(legend);
	}
}

void ConfSpaceView::applyRules()
{
	for (size_t i = 0; i < _temps.size(); i++)
	{
		removeObject(_temps[i]);
		delete _temps[i];
	}

	_temps.clear();
	
	IconLegend *il = new IconLegend();

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
