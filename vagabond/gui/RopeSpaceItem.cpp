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

#include "RopeSpaceItem.h"
#include "ConfSpaceView.h"
#include "Entity.h"
#include "Instance.h"
#include "Axes.h"
#include "RopeCluster.h"
#include "MetadataView.h"
#include <vagabond/core/Metadata.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>

using namespace rope;

RopeSpaceItem::RopeSpaceItem(Entity *entity) : Item()
{
	_entity = entity;
	setDisplayName("All molecules in " + entity->name());
	Item::setSelectable(true);
}

RopeSpaceItem::~RopeSpaceItem()
{
	std::cout << "DELETING ROPE SPACE ITEM?" << std::endl;
	_confView->removeResponder(_axes);
	_confView->removeResponder(_view);
	delete _view;
	delete _cluster;
	delete _axes;
}

void RopeSpaceItem::makeView(ConfSpaceView *attach)
{
	_confView = attach;
	allocateView(attach);
	calculateCluster();
}

void RopeSpaceItem::attachExisting(ConfSpaceView *attach)
{
	_confView = attach;

	attach->addIndexResponder(_view);
	_view->setIndexResponseView(attach);
	_view->setConfSpaceView(attach);
	
	if (_axes)
	{
		_axes->setScene(attach);
		attach->addIndexResponder(_axes);
		_axes->setIndexResponseView(attach);
	}
}

void RopeSpaceItem::allocateView(ConfSpaceView *attach)
{
	ClusterView *view = new ClusterView();
	_view = view;
	attachExisting(attach);
}

void RopeSpaceItem::calculateCluster()
{
	if (_type == ConfTorsions)
	{
		MetadataGroup angles = _entity->makeTorsionDataGroup();
		angles.setWhiteList(_whiteList);
		angles.write(_entity->name() + ".csv");
		angles.normalise();

		RopeCluster *cx = nullptr;
		cx = new TorsionCluster(angles);

		_cluster = cx;
		_view->setCluster(cx);
	}
	
	if (_type == ConfPositional)
	{
		PositionalGroup group = _entity->makePositionalDataGroup();
		group.setWhiteList(_whiteList);

		PositionalCluster *cx = nullptr;
		cx = new PositionalCluster(group);

		_cluster = cx;
		_view->setCluster(cx);
	}

}

RopeSpaceItem *RopeSpaceItem::branchFromRuleRange(const Rule *rule, float min,
                                                  float max)
{
	ObjectGroup *mg = _cluster->objectGroup();
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		Metadata::KeyValues kv = hm->metadata();
		
		if (kv.count(rule->header()) > 0)
		{
			float num = kv.at(rule->header()).number();
			
			if (num >= min && num <= max)
			{
				whiteList.push_back(hm);
			}
		}
	}

	std::string title = rule->header();
	title += " from " + f_to_str(min, 1) + " to " + f_to_str(max, 1);
	title += " (" + std::to_string(whiteList.size()) + ")";

	RopeSpaceItem *subset = new RopeSpaceItem(_entity);
	subset->setWhiteList(whiteList);
	subset->setDisplayName(title);
	subset->makeView(_confView);
	addItem(subset);

	subset->inheritAxis(this);
	
	return subset;
}

RopeSpaceItem *RopeSpaceItem::branchFromRule(Rule *rule, bool inverse)
{
	ObjectGroup *mg = _cluster->objectGroup();
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

	std::string title = (inverse ? "NOT " : "");
	title += rule->exactDesc() + " (";
	title += std::to_string(whiteList.size()) + ")";

	RopeSpaceItem *subset = new RopeSpaceItem(_entity);
	subset->setWhiteList(whiteList);
	subset->setDisplayName(title);
	subset->makeView(_confView);
	addItem(subset);
	
	subset->inheritAxis(this);
	
	return subset;
}

RopeSpaceItem *RopeSpaceItem::makeGroupFromSelected(bool inverse)
{
	ObjectGroup *mg = _cluster->objectGroup();
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);

		if (hm->isSelected() == !inverse)
		{
			whiteList.push_back(hm);
		}
	}

	std::string title = "hand selected ";
	title += inverse ? "inverse (" : "(";
	title += std::to_string(whiteList.size()) + ")";

	RopeSpaceItem *subset = new RopeSpaceItem(_entity);
	subset->setWhiteList(whiteList);
	subset->setDisplayName(title);
	subset->makeView(_confView);
	addItem(subset);

	subset->inheritAxis(this);
	
	return subset;
}

void RopeSpaceItem::inheritAxis(RopeSpaceItem *parent)
{
	if (_type != ConfTorsions || !parent->axes())
	{
		return;
	}

	ObjectGroup *mg = _cluster->objectGroup();
	Axes *old = parent->axes();
	HasMetadata *hm = old->instance();

	if (mg->indexOfObject(hm) >= 0)
	{
		TorsionCluster *tc = static_cast<TorsionCluster *>(_cluster);
		_axes = new Axes(tc, old->instance());
		_axes->takeOldAxes(old);
	}
}

Axes *RopeSpaceItem::createReference(Instance *inst)
{
	if (_type != ConfTorsions)
	{
		return nullptr;
	}

	Axes *old = _axes;
	if (_axes != nullptr)
	{
		_confView->removeObject(_axes);
		_confView->removeResponder(_axes);
	}
	
	TorsionCluster *tc = static_cast<TorsionCluster *>(_cluster);
	
	_axes = new Axes(tc, inst);
	_axes->setScene(_confView);
	_axes->setIndexResponseView(_confView);
	
	if (old)
	{
		_axes->takeOldAxes(old);
		delete old;
	}

	_confView->addObject(_axes);
	_confView->addIndexResponder(_axes);

	return _axes;
}

void RopeSpaceItem::deleteAxes()
{
	if (_axes)
	{
		_confView->removeResponder(_axes);
		_confView->removeObject(_axes);
		delete _axes;
		_axes = nullptr;
	}
}

Menu *RopeSpaceItem::rightClickMenu()
{
	Menu *m = new Menu(_confView, this);
	if (parent())
	{
		m->addOption("delete", "delete");
		m->addOption("set metadata", "set_metadata");
	}
	return m;
}

void RopeSpaceItem::buttonPressed(std::string tag, Button *button)
{
	std::string meta = Button::tagEnd(tag, "metadata_");
	
	if (meta.length())
	{
		handleMetadataTag(meta, button);
	}
	
	if (tag == "set_metadata")
	{
		changeMetadata();
	}

	if (tag == "delete")
	{
		deleteItem();
		_confView->showCurrentCluster();
	}

	resolveDeletions();
}

size_t RopeSpaceItem::selectedCount()
{
	ObjectGroup *mg = _cluster->objectGroup();
	size_t count = 0;
	
	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		
		if (hm->isSelected())
		{
			count++;
		}
	}
	
	return count;
}

int RopeSpaceItem::separateAverageCount()
{
	ObjectGroup *mg = _cluster->objectGroup();
	return mg->groupCount();
}

void RopeSpaceItem::handleMetadataTag(std::string tag, Button *button)
{
	TextEntry *entry = static_cast<TextEntry *>(button);
	if (tag == "key")
	{
		_tmpKey = entry->scratch();
		if (_tmpKey.length() == 0)
		{
			std::string msg = ("Metadata key is blank, cannot assign.");
			BadChoice *bc = new BadChoice(_confView, msg);
			_confView->setModal(bc);
			return;
		}

		AskForText *aft = new AskForText(_confView, "Metadata value to enter:",
		                                 "metadata_value", this);

		_confView->setModal(aft);
	}
	else if (tag == "value")
	{
		_tmpValue = entry->scratch();
		if (_tmpValue.length() == 0)
		{
			std::string msg = ("Metadata value is blank, cannot assign.");
			BadChoice *bc = new BadChoice(_confView, msg);
			_confView->setModal(bc);
			return;
		}
		setMetadata(_tmpKey, _tmpValue);
		_tmpValue = "";
		_tmpKey = "";
	}

}

void RopeSpaceItem::setMetadata(std::string key, std::string value)
{
	Metadata *md = new Metadata();
	ObjectGroup *mg = _cluster->objectGroup();
	size_t count = 0;
	
	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		Metadata::KeyValues kv;

		kv["molecule"] = Value(hm->id());
		kv[_tmpKey] = Value(_tmpValue);

		md->addKeyValues(kv, true);
	}

	MetadataView *mv = new MetadataView(_confView, md);
	mv->show();
}

void RopeSpaceItem::changeMetadata()
{
	AskForText *aft = new AskForText(_confView, "Metadata key to enter:",
	                                 "metadata_key", this);

	_confView->setModal(aft);
}
