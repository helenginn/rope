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
}

void ClusterView::updatePoints()
{
	if (vertexCount() > _cx->objectGroup()->objectCount())
	{
		_vertices.resize(_cx->objectGroup()->objectCount());
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 v = _cx->pointForDisplay(i);
		_vertices[i].pos = v;
	}
}

void ClusterView::additionalJobs()
{
	_invert = new std::thread(ClusterView::invertSVD, this);
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
	
	std::cout << "Vertices: " << vertexCount() << std::endl;
	
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

	ObjectGroup *group = _cx->objectGroup();
	std::vector<float> values = r.valuesForObjects(group);

	for (size_t i = 0; i < values.size(); i++)
	{
		glm::vec4 colour = cs->colour(values[i]);
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

void ClusterView::respond()
{
	wait();
}

void ClusterView::waitForInvert()
{
	if (_invert)
	{
		_invert->join();
		delete _invert;
		_invert = nullptr;
	}
}

void ClusterView::wait()
{
	if (_worker)
	{
		passiveWait();
		_worker->join();
		delete _worker;
		_worker = nullptr;
		_cv.notify_all();
	}
}

void ClusterView::setFinish(bool finish)
{
	std::unique_lock<std::mutex> lock(_lockPopulating);
	_finish = finish;
}
	
void ClusterView::passiveWait()
{
	std::unique_lock<std::mutex> lock(_lockPopulating);
	if (!_running)
	{
		return;
	}

	_finish = true;
	_cv.wait(lock);
}

void ClusterView::invertSVD(ClusterView *me)
{
	if (me->_cx)
	{
		int myVersion = me->_clusterVersion;
		me->_clusterVersion = me->_cx->version();
		
		if (myVersion < me->_clusterVersion)
		{
			me->_cx->calculateInverse();
		}
	}

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


