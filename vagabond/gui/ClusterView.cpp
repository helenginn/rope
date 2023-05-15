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

#include "ClusterView.h"
#include "ColourScheme.h"
#include "ConfSpaceView.h"
#include <vagabond/gui/PathView.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/gui/elements/FloatingText.h>

#include <vagabond/core/Rule.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/c4x/ClusterSVD.h>
#include <vagabond/core/ObjectGroup.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/RopeCluster.h>

#include <iostream>

ClusterView::ClusterView() : PointyView()
{
	setName("ClusterView");
	
#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
}

ClusterView::~ClusterView()
{
	wait();
	deleteObjects();
}

void ClusterView::updatePoints()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 v = _cx->pointForDisplay(i);
		_vertices[i].pos = v;
	}
}

void ClusterView::additionalJobs()
{
	_invert = new std::thread(ClusterView::invertSVD, this);
	addPaths();
}

void ClusterView::prioritiseMetadata(std::string key)
{
	std::vector<float> vals = _cx->objectGroup()->numbersForKey(key);
	_cx->chooseBestAxes(vals);

	refresh();
}

void ClusterView::makePoints()
{
	if (_cx == nullptr)
	{
		return;
	}

	_cx->cluster();
	clearVertices();
	
	size_t count = _cx->pointCount();
	_vertices.reserve(count);
	_indices.reserve(count);

	for (size_t i = 0; i < count; i++)
	{
		if (!_cx->objectGroup()->object(i)->displayable())
		{
			continue;
		}

		glm::vec3 v = _cx->pointForDisplay(i);

		_point2Index[vertexCount()] = i;

		addPoint(v, 0);
	}
	
	reindex();
}

void ClusterView::setCluster(RopeCluster *cx)
{
	_cx = cx;

	makePoints();
}

void ClusterView::applySelected()
{
	ObjectGroup &group = *_cx->objectGroup();
	for (size_t i = 0; i < group.objectCount(); i++)
	{
		HasMetadata *hm = group.object(i);
		if (hm->isSelected())
		{
			_vertices[i].color = glm::vec4(1.0, 1.0, 0.1, 1.0);
		}
	}
	
	forceRender();
}

void ClusterView::applyVaryColour(const Rule &r)
{
	std::string header = r.header();
	Scheme sch = r.scheme();
	ColourScheme *cs = new ColourScheme(sch);
	
	if (cs == nullptr)
	{
		return;
	}

	ObjectGroup &group = *_cx->objectGroup();
	for (size_t i = 0; i < group.objectCount(); i++)
	{
		const Metadata::KeyValues data = group.object(i)->metadata();
		
		if (data.count(header) == 0 || !data.at(header).hasNumber())
		{
			_vertices[i].color = glm::vec4(0.5, 0.5, 0.5, 0.2);
			continue;
		}
		float val = data.at(header).number();
		r.convert_value(val);
		glm::vec4 colour = cs->colour(val);
		_vertices[i].color = colour;
	}
	
	forceRender();

	delete cs;
}

void ClusterView::applyChangeIcon(const Rule &r)
{
	int pt = r.pointType();

	ObjectGroup &group = *_cx->objectGroup();
	for (size_t i = 0; i < group.objectCount(); i++)
	{
		if (r.appliesToObject(group.object(i)))
		{
			setPointType(i, pt);
			_members[&r].push_back(group.object(i));
		}
	}
	
	forceRender();
}

void ClusterView::applyRule(const Rule &r)
{
	if (r.type() == Rule::VaryColour)
	{
		applyVaryColour(r);
	}
	else if (r.type() == Rule::ChangeIcon)
	{
		applyChangeIcon(r);
	}
}

void ClusterView::click(bool left)
{
	interacted(currentVertex(), false, left);
}

void ClusterView::interacted(int rawidx, bool hover, bool left)
{
	if (_text != nullptr)
	{
		removeObject(_text);
		delete _text;
		_text = nullptr;
	}
	
	if (rawidx < 0 || rawidx >= _vertices.size())
	{
		return;
	}
	
	int idx = _point2Index[rawidx];
	
	ObjectGroup &group = *_cx->objectGroup();

	if (_confSpaceView->returnToView() && left && !hover)
	{
		Polymer *pol = static_cast<Polymer *>(group.object(idx));
		_confSpaceView->reorientToPolymer(pol);
		return;
	}

	std::string str = group.object(idx)->id();

	FloatingText *ft = new FloatingText(str);
	ft->setPosition(_vertices[rawidx].pos);

	addObject(ft);
	_text = ft;
	
	if (hover == false && !left) // click!
	{
		_confSpaceView->prepareModelMenu(group.object(idx));
	}
}

void ClusterView::clearPaths()
{
	for (PathView *pv : _pathViews)
	{
		removeObject(pv);
		delete pv;
	}

	_pathViews.clear();
}

void ClusterView::addPathView(PathView *pv)
{
	_pathViews.push_back(pv);
	_path2View[pv->path()] = pv;
	addObject(pv);
}

void ClusterView::respond()
{
	wait();
	clearPaths();
}

void ClusterView::addPath(Path *path, bool populate)
{
	if (!path->visible())
	{
		return;
	}

	if (_confSpaceView && _confSpaceView->entity() 
	    != path->startInstance()->entity())
	{
		return;
	}

	TorsionCluster *svd = nullptr;
	svd = static_cast<TorsionCluster *>(_cx);
	
	for (auto it = _path2View.begin(); it != _path2View.end(); it++)
	{
		if (it->first->sameRouteAsPath(path))
		{
			PathView *old = it->second;
			auto jt = std::find(_pathViews.begin(), _pathViews.end(), old);
			_pathViews.erase(jt);

			removeObject(old);
			Window::setDelete(old);
			_path2View.erase(it);
			break;
		}
	}

	PathView *pv = new PathView(*path, svd);
	
	if (populate)
	{
		pv->populate();
	}

	addPathView(pv);
}

void ClusterView::addPaths()
{
	wait();
	clearPaths();
	
	if (_confSpaceView && _confSpaceView->confType() != rope::ConfTorsions)
	{
		return;
	}

	PathManager *pm = Environment::env().pathManager();

	for (Path &path : pm->objects())
	{
		addPath(&path, false);
	}
	
	VagWindow *window = VagWindow::window();
	window->prepareProgressBar(_pathViews.size(), "Loading paths");
	_worker = new std::thread(ClusterView::populatePaths, this);
}

void ClusterView::wait()
{
	if (_worker)
	{
		_finish = true;
		_worker->join();
		delete _worker;
		_worker = nullptr;
		_finish = false;
	}
}

void ClusterView::invertSVD(ClusterView *me)
{
	if (me->_cx)
	{
		me->_cx->calculateInverse();
	}

}

void ClusterView::populatePaths(ClusterView *me)
{
	for (PathView *pv : me->_pathViews)
	{
		pv->populate();
		
		me->clickTicker();
		if (me->_finish)
		{
			break;
		}
	}

	PathManager *pm = Environment::env().pathManager();

	for (Path &path : pm->objects())
	{
//		path.cleanupRoute();
	}

	me->finishTicker();
}

void ClusterView::selected(int rawidx, bool inverse)
{
	if (rawidx < 0 || rawidx >= _vertices.size())
	{
		return;
	}
	
	int idx = _point2Index[rawidx];
	
	ObjectGroup &group = *_cx->objectGroup();
	HasMetadata *hm = static_cast<HasMetadata *>(group.object(idx));

	if (hm)
	{
		hm->setSelected(!inverse);
	}
}

void ClusterView::deselect()
{
	ObjectGroup &group = *_cx->objectGroup();

	for (size_t i = 0; i < group.objectCount(); i++)
	{
		HasMetadata *hm = static_cast<HasMetadata *>(group.object(i));
		hm->setSelected(false);
	}
}

std::vector<HasMetadata *> ClusterView::selectedMembers()
{
	std::vector<HasMetadata *> hms;

	ObjectGroup &group = *_cx->objectGroup();

	for (size_t i = 0; i < group.objectCount(); i++)
	{
		HasMetadata *hm = static_cast<HasMetadata *>(group.object(i));
		if (hm->isSelected())
		{
			hms.push_back(hm);
		}
	}
	
	return hms;
}
