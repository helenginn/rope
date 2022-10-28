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
class Molecule;
class Rule;
class Axes;
class HasMetadata;
class MetadataGroup;
class RouteExplorer;
class PathView;

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
	
	void setTSNE(bool tsne)
	{
		_tsne = tsne;
	}
	
	Entity *entity()
	{
		return _entity;
	}

	virtual void setup();
	virtual void refresh();
	virtual void doThings();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	
	void prepareMenu(HasMetadata *hm);
	void reorientToMolecule(Molecule *mol);

	void sendObject(std::string tag, void *object);
protected:
	virtual void sendSelection(float t, float l, float b, float r);

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
	void createReference(Molecule *m);

	void askToFoldIn(int extra);
	Entity *_entity = nullptr;
	int _extra = 0;
	ClusterView *_view = nullptr;
	Axes *_axes = nullptr;
	const Rule *_colourRule = nullptr;
	
	Molecule *_from = nullptr;
	RouteExplorer *_routeExplorer = nullptr;

	std::vector<HasMetadata *> _whiteList;
	std::vector<Renderable *> _temps;
	
	Status _status = Nothing;

	bool _shownPaths = false;
	bool _tsne = false;
};

#endif

