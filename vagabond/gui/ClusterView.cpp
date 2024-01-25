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
	if (vertexCount() > _data->objectCount())
	{
		_vertices.resize(_data->objectCount());
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
	std::vector<float> vals = _data->numbersForKey(key);
	_cx->chooseBestAxes(vals);

	refresh();
}

void ClusterView::makePoints()
{
	if (_cx == nullptr)
	{
		return;
	}

	clearVertices();
	
	size_t count = _data->objectCount();
	_vertices.reserve(count);
	_indices.reserve(count);

	for (size_t i = 0; i < count; i++)
	{
		if (!_data->object(i)->displayable())
		{
			continue;
		}

		glm::vec3 v = _cx->pointForDisplay(i);

		_point2Index[vertexCount()] = i;

		addPoint(v, 0);
	}
	
	reindex();
}

void ClusterView::setCluster(ClusterSVD *cx, ObjectGroup *data)
{
	_data = data;
	_cx = cx;

	makePoints();
}

void ClusterView::applySelected()
{
	for (size_t i = 0; i < _data->objectCount(); i++)
	{
		HasMetadata *hm = _data->object(i);
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

	std::vector<float> values = r.valuesForObjects(_data);

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

	for (size_t i = 0; i < _data->objectCount(); i++)
	{
		if (r.appliesToObject(_data->object(i)))
		{
			setPointType(i, pt);
			_members[&r].push_back(_data->object(i));
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

	if (_confSpaceView->returnToView() && left && !hover)
	{
		Polymer *pol = static_cast<Polymer *>(_data->object(idx));
		_confSpaceView->reorientToPolymer(pol);
		return;
	}

	std::string str = _data->object(idx)->id();

	FloatingText *ft = new FloatingText(str);
	ft->setPosition(_vertices[rawidx].pos);

	addObject(ft);
	_text = ft;
	
	if (hover == false && !left) // click!
	{
		_confSpaceView->prepareModelMenu(_data->object(idx));
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
	
	HasMetadata *hm = static_cast<HasMetadata *>(_data->object(idx));

	if (hm)
	{
		hm->setSelected(!inverse);
	}
}

void ClusterView::deselect()
{
	for (size_t i = 0; i < _data->objectCount(); i++)
	{
		HasMetadata *hm = static_cast<HasMetadata *>(_data->object(i));
		hm->setSelected(false);
	}
}

std::vector<HasMetadata *> ClusterView::selectedMembers()
{
	std::vector<HasMetadata *> hms;

	for (size_t i = 0; i < _data->objectCount(); i++)
	{
		HasMetadata *hm = static_cast<HasMetadata *>(_data->object(i));
		if (hm->isSelected())
		{
			hms.push_back(hm);
		}
	}
	
	return hms;
}


