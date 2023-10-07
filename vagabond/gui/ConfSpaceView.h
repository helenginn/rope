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

#ifndef __vagabond__ConfSpaceView__
#define __vagabond__ConfSpaceView__

#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/Responder.h>
#include <vagabond/core/ConfType.h>
#include "ClusterView.h"

class Entity;
class Polymer;
class Rule;
class Axes;
class HasMetadata;
class MetadataGroup;
class RouteExplorer;
class RopeSpaceItem;
class PathView;

template <class DG>
class Cluster;

class ConfSpaceView : public Mouse3D, public IndexResponseView,
public Responder<PathView>
{
public:
	ConfSpaceView(Scene *prev, Entity *ent);
	~ConfSpaceView();

	rope::ConfType confType()
	{
		return _type;
	}
	
	void setMode(rope::ConfType type)
	{
		_type = type;
	}
	
	Entity *entity()
	{
		return _entity;
	}
	
	const Rule *colourRule() const
	{
		return _colourRule;
	}
	
	ClusterView *view()
	{
		return _view;
	}

	virtual void setup();
	virtual void refresh();
	virtual void doThings();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	
	void prepareModelMenu(HasMetadata *hm);
	void reorientToPolymer(Polymer *mol);

	void showCurrentCluster();
	void sendObject(std::string tag, void *object);

	void assignRopeSpace(RopeSpaceItem *item);
	
	void setMadePaths()
	{
		_madePaths = true;
	}
protected:
	virtual void sendSelection(float t, float l, float b, float r,
	                           bool inverse);
	virtual void interactedWithNothing(bool left);
	void prepareEmptySpaceMenu();

private:
	enum Status
	{
		Nothing,
		Reorienting,
	};
	
public:
	bool returnToView()
	{
		return (_status == Reorienting);
	}
private:
	void makeFirstCluster();
	void chooseGroup(Rule *rule, bool inverse);
	void grabSelection(bool inverse);
	void executeSubset(float min, float max);
	void switchView();
	void proofRopeSpace();
	void displayTree();

	void showPathsButton();
	void showRulesButton();
	void showAxesButton();
	void addGuiElements();

	void applyRule(const Rule &r);
	void applyRules();
	void removeRules();
	void createReference(Instance *i);

	void askToFoldIn(int extra);
	
	RopeSpaceItem *const savedSpace() const
	{
		if (_savedSpaces.count(_entity) == 0 ||
		    _savedSpaces.at(_entity).count(_type) == 0)
		{
			return nullptr;
		}

		return _savedSpaces.at(_entity).at(_type);
	}
	
	void setSavedSpace(RopeSpaceItem *item)
	{
		_savedSpaces[_entity][_type] = item;
	}
	
	static std::map<Entity *, 
	std::map<rope::ConfType, RopeSpaceItem *>> _savedSpaces;
	
	RopeSpaceItem *_ropeSpace = nullptr;
	RopeSpaceItem *_selected = nullptr;
	LineGroup *_ropeTree = nullptr;

	Entity *_entity = nullptr;
	int _extra = 0;
	ClusterView *_view = nullptr;
	RopeCluster *_cluster = nullptr;
	Axes *_axes = nullptr;
	Axes *_origin = nullptr;
	const Rule *_colourRule = nullptr;
	rope::ConfType _type = rope::ConfTorsions;
	
	Polymer *_from = nullptr;
	RouteExplorer *_routeExplorer = nullptr;
	
	Status _status = Nothing;

	static bool _madePaths;
	bool _shownPathsButton = false;
};

#endif

