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
#include <vagabond/core/Metadata.h>
#include <vagabond/core/PathData.h>
#include <vagabond/core/TorsionData.h>
#include <vagabond/core/PositionData.h>
#include <vagabond/core/BFactorData.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>

#include "ConfSpaceView.h"
#include "Entity.h"
#include "Instance.h"
#include "Axes.h"
#include "AxisRequests.h"
#include "MetadataView.h"
#include "TorsionData.h"
#include "PositionData.h"
#include "PathManager.h"

#include "RopeSpaceItem.h"

using namespace rope;

RopeSpaceItem::RopeSpaceItem(Entity *entity) : Item()
{
	_entity = entity;
	_fixedTitle = "All molecules in " + entity->name();
	Item::setSelectable(true);
}

RopeSpaceItem::~RopeSpaceItem()
{
	_confView->removeResponder(_axes);
	_confView->removeResponder(_view);
	delete _view;
	delete _cluster;
	delete _axes;
	delete _group;
}

void RopeSpaceItem::makeView(ConfSpaceView *attach)
{
	setConfView(attach);
	allocateView();
	calculateCluster();
	attachExisting(attach);
}

void RopeSpaceItem::setConfView(ConfSpaceView *attach)
{
	for (size_t i = 0; i < itemCount(); i++)
	{
		ropeSpaceItem(i)->setConfView(attach);
	}
	
	_confView = attach;
}

void RopeSpaceItem::clusterIfNeeded()
{
	if (!_mustCluster)
	{
		return;
	}

	_confView->assignRopeSpace(nullptr);
	_cluster->setFilterHeader(setupResidueFilter());
	_cluster->cluster();
	_view->setCluster(_cluster, _group);

	_mustCluster = false;
}

void RopeSpaceItem::attachExisting(ConfSpaceView *attach)
{
	ClusterView *oldView = nullptr;
	
	if (_confView)
	{
		oldView = _confView->view();
	}

	setConfView(attach);

	clusterIfNeeded();
	_confView->assignRopeSpace(this);

	attach->addIndexResponder(_view);
	_view->setIndexResponseView(attach);
	_view->setConfSpaceView(attach);
	
	if (_axes)
	{
		_axes->setScene(attach);
		attach->addIndexResponder(_axes);
		_axes->setIndexResponseView(attach);
	}

	_view->setInherit(oldView);
	_view->additionalJobs();
}

void RopeSpaceItem::allocateView()
{
	if (_view)
	{
		return;
	}

	ClusterView *view = new ClusterView();
	_view = view;
}

void RopeSpaceItem::purgeHasMetadata(HasMetadata *hm)
{
	bool found = _group->purgeObject(hm);
	
	if (found)
	{
		setMustCluster();
	}
	
	std::vector<HasMetadata *> objects = _group->objects();
	auto it = std::find(objects.begin(), objects.end(), hm);
	
	if (it != objects.end())
	{
		objects.erase(it);
	}

	_group->setWhiteList(objects);
	
	for (int i = 0; i < itemCount(); i++)
	{
		RopeSpaceItem *rsi = ropeSpaceItem(i);
		rsi->purgeHasMetadata(hm);
	}
}

void RopeSpaceItem::sendObject(std::string tag, void *object)
{
	if (tag == "purge")
	{
		HasMetadata *hm = static_cast<HasMetadata *>(object);
		purgeHasMetadata(hm);
	}

}

void RopeSpaceItem::setResponders()
{
	if (!isTopLevelItem() || !_entity)
	{
		return;
	}

	std::vector<Instance *> insts = _entity->instances();
	
	for (Instance *inst : insts)
	{
		inst->setResponder(this);
	}
}

template <class ExactGroup>
ObjectData *prepareCopy(const ExactGroup &other)
{
	ExactGroup *grp = new ExactGroup(other);
	grp->setDoEditMenu(editMenu(grp));
	grp->setDoRequest(doRequest(grp));

	return grp;
}

ObjectData *RopeSpaceItem::appropriateStartingGroup()
{
	ObjectData *grp = nullptr;
	if (_entity == nullptr)
	{
		throw std::runtime_error("Can't produce starting group - no group,"\
		                         " no entity");
	}

	if (_type == ConfTorsions)
	{
		grp = prepareCopy(_entity->makeTorsionDataGroup());
	}
	else if (_type == ConfPositional)
	{
		grp = prepareCopy(_entity->makePositionalDataGroup());
	}
	else
	{
		grp = prepareCopy(_entity->makeBFactorGroup());
	}
	
	return grp;
}

void RopeSpaceItem::prepareCluster()
{
	allocateView();

	if (_group == nullptr)
	{
		_group = appropriateStartingGroup();
		std::string str;
		if (_entity)
		{
			str += _entity->name() + "_";
		}
		str += tag_for_conf_type(_type);
		str += ".csv";
		_group->write_data(str);
	}

	_cluster = new ClusterSVD(_group->data());
	_view->setCluster(_cluster, _group);
}

void RopeSpaceItem::calculateCluster()
{
	if (_cluster != nullptr)
	{
		return;
	}

	setResponders();
	prepareCluster();
}

RopeSpaceItem *RopeSpaceItem::branchFromRuleRange(const Rule *rule, float min,
                                                  float max)
{
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);
		Metadata::KeyValues kv = hm->metadata(_md);
		
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

	RopeSpaceItem *subset = newFrom(whiteList, title);
	
	return subset;
}

RopeSpaceItem *RopeSpaceItem::branchFromRule(Rule *rule, bool inverse)
{
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);
		Metadata::KeyValues kv = hm->metadata(_md);
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
	title += rule->exactDesc();

	RopeSpaceItem *subset = newFrom(whiteList, title);
	return subset;
}

std::string RopeSpaceItem::displayName() const
{
	std::string title = _fixedTitle;
	
	if (!isTopLevelItem())
	{
		title += " (" + std::to_string(_group->objectCount()) + ")";
	}

	return title;
}

RopeSpaceItem *RopeSpaceItem::makeGroupFromSelected(bool inverse)
{
	std::vector<HasMetadata *> whiteList;

	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);

		if (hm->isSelected() == !inverse)
		{
			whiteList.push_back(hm);
		}
	}

	std::string title = "hand selected";
	title += inverse ? " inverse" : "";

	RopeSpaceItem *subset = newFrom(whiteList, title);
	return subset;
}

RopeSpaceItem *RopeSpaceItem::newFrom(std::vector<HasMetadata *> &whiteList,
                                      std::string title)
{
	ObjectData *group_ptr = nullptr;
	
	if (_type == ConfTorsions)
	{
		TorsionData *group = static_cast<TorsionData *>(_group);
		group_ptr = prepareCopy(*group);
	}
	else if (_type == ConfPositional)
	{
		PositionData *group = static_cast<PositionData *>(_group);
		group_ptr = prepareCopy(*group);
	}
	else if (_type == ConfBFactor)
	{
		BFactorData *group = static_cast<BFactorData *>(_group);
		group_ptr = prepareCopy(*group);
	}
	else if (_type == ConfPath)
	{
		PathData *group = static_cast<PathData *>(_group);
		group_ptr = prepareCopy(*group);
	}

	group_ptr->setWhiteList(whiteList);
	RopeSpaceItem *subset = new RopeSpaceItem(_entity);
	subset->setMode(_type);
	subset->setFixedTitle(title);
	subset->setObjectGroup(group_ptr);
	subset->setMetadata(_md);
	addItem(subset);
	subset->makeView(_confView);

	subset->inheritAxis(this);

	return subset;
}

void RopeSpaceItem::inheritAxis(RopeSpaceItem *parent)
{
	if (_type != ConfTorsions || !parent->axes())
	{
		return;
	}

	Axes *old = parent->axes();
	HasMetadata *hm = old->focus();

	if (_group->indexOfObject(hm) >= 0)
	{
		_axes = new Axes(_group, _cluster, old->focus());
		_axes->takeOldAxes(old);
	}
}

Axes *RopeSpaceItem::createReference(Instance *inst)
{
	Axes *old = _axes;
	if (_axes != nullptr)
	{
		_confView->removeObject(_axes);
		_confView->removeResponder(_axes);
	}
	
	_axes = new Axes(_group, _cluster, inst);

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
	size_t count = 0;
	
	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);
		
		if (hm->isSelected())
		{
			count++;
		}
	}
	
	return count;
}

int RopeSpaceItem::separateAverageCount()
{
	return _group->numGroups();
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
	
	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);
		Metadata::KeyValues kv;

		kv["instance"] = Value(hm->id());
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

RopeSpaceItem *RopeSpaceItem::ropeSpaceItem(int idx)
{
	return static_cast<RopeSpaceItem *>(item(idx));
}

std::function<bool(int)> RopeSpaceItem::setupResidueFilter()
{
	auto header_check = [this](int i)
	{
		RegionManager *manager = &_entity->regionManager();
		BitIdentifier bit{};
		if (_type == ConfTorsions)
		{
			TorsionData *group = static_cast<TorsionData *>(_group);
			bit = group->headers()[i];
		}
		else if (_type == ConfPositional)
		{
			PositionData *group = static_cast<PositionData *>(_group);
			bit = group->headers()[i];
		}
		else
		{
			return true;
		}

		bool accept = manager->residueIsAcceptable(bit.id());
		return accept;
	};
	
	return header_check;
}


void RopeSpaceItem::setMustCluster(bool recursive)
{
	_mustCluster = true;
	
	if (!recursive) return;
	
	for (size_t i = 0; i < itemCount(); i++)
	{
		ropeSpaceItem(i)->setMustCluster(recursive);
	}
}
