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

#include <vagabond/gui/elements/list/Item.h>
#include <vagabond/core/ConfType.h>

class HasMetadata;
class ClusterView;
class RopeCluster;
class Instance;
class Entity;
class Axes;
class Rule;
	
class ConfSpaceView;

class RopeSpaceItem : public Item
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
	
	rope::ConfType confType()
	{
		return _type;
	}

	void setMode(rope::ConfType type)
	{
		_type = type;
	}
	
	void makeView(ConfSpaceView *attach);
	
	Axes *axes()
	{
		return _axes;
	}
	
	Axes *createReference(Instance *inst);
	RopeSpaceItem *branchFromRule(Rule *rule, bool inverse);
	RopeSpaceItem *branchFromRuleRange(const Rule *rule, float min, float max);
	
	void deleteAxes();
private:
	void allocateView(ConfSpaceView *view);
	void inheritAxis(RopeSpaceItem *parent);
	void calculateCluster();

	std::vector<HasMetadata *> _whiteList;
	ClusterView *_view = nullptr;
	RopeCluster *_cluster = nullptr;
	Axes *_axes = nullptr;
	Entity *_entity = nullptr;
	ConfSpaceView *_confView = nullptr;

	rope::ConfType _type = rope::ConfTorsions;
};

#endif
