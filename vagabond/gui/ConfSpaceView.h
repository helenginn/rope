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
#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/Responder.h>
#include "ClusterView.h"

class Entity;
class Polymer;
class Rule;
class Axes;
class HasMetadata;
class MetadataGroup;
class RouteExplorer;
class PathView;

template <class DG>
class Cluster;

class ConfSpaceView : public Mouse3D, public IndexResponseView,
public Responder<PathView>
{
public:
	ConfSpaceView(Scene *prev, Entity *ent);
	~ConfSpaceView();

	void setWhiteList(std::vector<HasMetadata *> whiteList)
	{
		_whiteList = whiteList;
	}
	
	enum ConfType
	{
		ConfPositional,
		ConfTorsions,
	};
	
	ConfType confType()
	{
		return _type;
	}
	
	void setMode(ConfType type)
	{
		_type = type;
	}
	
	void setTSNE(bool tsne)
	{
		_tsne = tsne;
	}
	
	Entity *entity()
	{
		return _entity;
	}
	
	const Rule *colourRule() const
	{
		return _colourRule;
	}

	virtual void setup();
	virtual void refresh();
	virtual void doThings();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	
	void prepareModelMenu(HasMetadata *hm);
	void reorientToPolymer(Polymer *mol);

	void sendObject(std::string tag, void *object);
protected:
	virtual void sendSelection(float t, float l, float b, float r);
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
	void chooseGroup(Rule *rule, bool inverse);
	void executeSubset(float min, float max);
	void showClusters();

	void showPathsButton();
	void showRulesButton();
	void showAxesButton();
	void showtSNE();
	void addGuiElements();

	void applyRule(const Rule &r);
	void applyRules();
	void removeRules();
	void createReference(Polymer *m);

	void askToFoldIn(int extra);
	Entity *_entity = nullptr;
	int _extra = 0;
	ClusterView *_view = nullptr;
	RopeCluster *_cluster = nullptr;
	Axes *_axes = nullptr;
	Axes *_origin = nullptr;
	const Rule *_colourRule = nullptr;
	
	Polymer *_from = nullptr;
	RouteExplorer *_routeExplorer = nullptr;

	std::vector<HasMetadata *> _whiteList;
	std::vector<Renderable *> _temps;
	
	Status _status = Nothing;
	ConfType _type = ConfTorsions;

	bool _shownPaths = false;
	bool _tsne = false;
};

#endif

