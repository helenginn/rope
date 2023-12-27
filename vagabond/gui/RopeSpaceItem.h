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

#ifndef __vagabond__RopeSpaceItem__
#define __vagabond__RopeSpaceItem__

#include <vagabond/core/Item.h>
#include <vagabond/gui/elements/ButtonResponder.h>
#include <vagabond/core/ConfType.h>


class ButtonResponder;
class MetadataGroup;
class HasMetadata;
class ClusterView;
class RopeCluster;
class Instance;
class Entity;
class Axes;
class Rule;
	
class ConfSpaceView;

class RopeSpaceItem : public Item, public ButtonResponder, 
public Responder<HasMetadata>
{
public:
	RopeSpaceItem(Entity *entity);
	virtual ~RopeSpaceItem();

	void setWhiteList(std::vector<HasMetadata *> &wl)
	{
		_whiteList = wl;
	}
	
	ClusterView *view()
	{
		return _view;
	}
	
	RopeCluster *cluster()
	{
		return _cluster;
	}
	
	rope::ConfType confType()
	{
		return _type;
	}

	void setMode(rope::ConfType type)
	{
		_type = type;
	}
	
	virtual std::string displayName() const;
	
	void changeMetadata();
	
	int separateAverageCount();
	
	void makeView(ConfSpaceView *attach);
	void attachExisting(ConfSpaceView *attach);
	
	Axes *axes()
	{
		return _axes;
	}
	
	void setMustCluster()
	{
		_mustCluster = true;
	}
	
	Axes *createReference(Instance *inst);
	RopeSpaceItem *branchFromRule(Rule *rule, bool inverse);
	RopeSpaceItem *branchFromRuleRange(const Rule *rule, float min, float max);
	RopeSpaceItem *makeGroupFromSelected(bool inverse);
	void setConfView(ConfSpaceView *attach);
	
	size_t selectedCount();
	void torsionCluster(MetadataGroup *group = nullptr);
	virtual void sendObject(std::string tag, void *object);
	virtual void buttonPressed(std::string tag, Button *button);
	void deleteAxes();
	virtual Menu *rightClickMenu();
protected:
private:
	void setFixedTitle(const std::string &title)
	{
		_fixedTitle = title;
	}

	RopeSpaceItem *newFrom(std::vector<HasMetadata *> &whiteList,
	                       std::string title);

	void allocateView();
	void inheritAxis(RopeSpaceItem *parent);
	void calculateCluster();
	void setResponders();
	void handleMetadataTag(std::string tag, Button *button);
	void setMetadata(std::string key, std::string value);
	void purgeHasMetadata(HasMetadata *hm);
	void clusterIfNeeded();
	
	RopeSpaceItem *ropeSpaceItem(int idx);

	std::vector<HasMetadata *> _whiteList;
	ClusterView *_view = nullptr;
	RopeCluster *_cluster = nullptr;
	Axes *_axes = nullptr;
	Entity *_entity = nullptr;
	ConfSpaceView *_confView = nullptr;
	
	std::string _tmpKey, _tmpValue;
	bool _mustCluster = true;
	std::string _fixedTitle;

	rope::ConfType _type = rope::ConfTorsions;
};

#endif
