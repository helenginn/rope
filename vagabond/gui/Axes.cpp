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

#define _USE_MATH_DEFINES
#include <math.h>
#include <vagabond/utils/version.h>
#include <vagabond/core/RopeCluster.h>
#include <vagabond/core/ChemotaxisEngine.h>
#include <vagabond/core/Instance.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
#include "AxisExplorer.h"
#include "ConfSpaceView.h"
#include "PlausibleRoute.h"
#include "SplitRoute.h"
#include "RouteExplorer.h"
#include "Axes.h"

Axes::Axes(TorsionCluster *group, Instance *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_torsionCluster = group;
	_instance = m;
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	if (m) { setSelectable(true); }
#endif
}

Axes::Axes(PositionalCluster *group, Instance *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_positionalCluster = group;
	_instance = m;
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	if (m) { setSelectable(true); }
#endif
}

Axes::Axes(RopeCluster *group, Instance *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_instance = m;
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	if (m) { setSelectable(true); }
#endif
}

void Axes::initialise()
{
	setName("Axes");
	setImage("assets/images/axis.png");
	setUsesProjection(true);
	_searchType = Point;
	
	_dirs.push_back(glm::vec3(1., 0., 0.));
	_dirs.push_back(glm::vec3(0., 1., 0.));
	_dirs.push_back(glm::vec3(0., 0., 1.));

	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	
	for (size_t i = 0; i < 3; i++)
	{
		_targets[i] = nullptr;
		_planes[i] = false;
	}
}

void Axes::stop()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

}

Axes::~Axes()
{
	stop();
}

bool Axes::mouseOver()
{
	int axis = currentVertex() / 4;
	interacted(axis, true);
	return (currentVertex() >= 0);
}

void Axes::unmouseOver()
{
	interacted(-1, true);
}

void Axes::click(bool left)
{
	int axis = currentVertex() / 4;
	interacted(axis, false, left);
}

std::vector<float> Axes::getMappedVector(int idx)
{
	std::vector<float> vals(_cluster->rows(), 0);
	for (size_t i = 0; i < 3; i++)
	{
		vals[i] = _dirs[idx][i];
	}
	return vals;
}

std::vector<Angular> Axes::directTorsionVector(int idx)
{
	if (_torsionCluster == nullptr)
	{
		return std::vector<Angular>();
	}

	if (_targets[idx] != nullptr)
	{
		int mine = _torsionCluster->dataGroup()->indexOfObject(_instance);
		int yours = _torsionCluster->dataGroup()->indexOfObject(_targets[idx]);

		std::vector<Angular> vals = _torsionCluster->rawVector(mine, yours);
		return vals;
	}

	return getTorsionVector(idx);
}

std::vector<Angular> Axes::getTorsionVector(int idx)
{
	glm::vec3 dir = _dirs[idx];
	
	std::vector<Angular> sums;
	for (size_t i = 0; i < 3; i++)
	{
		if (i >= _torsionCluster->rows())
		{
			continue;
		}

		int axis = _torsionCluster->axis(i);
		std::vector<Angular> vals = _torsionCluster->rawVector(axis);

		float &weight = dir[i];
		std::cout << weight << std::endl;
		
		if (sums.size() == 0)
		{
			sums.resize(vals.size());
		}
		
		for (size_t j = 0; j < vals.size(); j++)
		{
			sums[j] += vals[j] * weight;
		}
	}
	
	return sums;
}

void Axes::loadAxisExplorer(int idx)
{
	std::vector<ResidueTorsion> list = _torsionCluster->dataGroup()->headers();

	std::vector<Angular> vals = getTorsionVector(idx);
	if (vals.size() == 0)
	{
		return;
	}
	
	std::string str = "Reference " + _instance->id();
	std::string info;
	
	if (_targets[idx] != nullptr)
	{
		str += " axis to target " + _targets[idx]->id();
	}
	else
	{
		int axis = _torsionCluster->axis(idx);
		str += " PCA axis " + i_to_str(axis);
	}

	try
	{
		AxisExplorer *ae = new AxisExplorer(_scene, _instance, list, vals);
		ae->setCluster(_torsionCluster);
		ae->setFutureTitle(str);
		ae->show();
//		ae->setInformation(info);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << err.what() << std::endl;
	}
}

void Axes::route(int idx)
{
	int l = _torsionCluster->dataGroup()->length();
	SplitRoute *sr = new SplitRoute(_instance, _torsionCluster, l);
	
	std::vector<Angular> values = directTorsionVector(idx);
	sr->setRawDestination(values);
	sr->setDestinationInstance(_targets[idx]);

	RouteExplorer *re = new RouteExplorer(_scene, sr);
	re->show();

}

void Axes::buttonPressed(std::string tag, Button *button)
{
	if (tag == "explore_axis")
	{
		loadAxisExplorer(_lastIdx);
	}
	else if (tag == "reorient")
	{
		_scene->buttonPressed("choose_reorient_molecule", nullptr);
	}
	else if (tag == "match_colour")
	{
		_scene->buttonPressed(tag, nullptr);
	}
	else if (tag == "reflect")
	{
		reflect(_lastIdx);
	}
	else if (tag == "reset")
	{
		reorient(_lastIdx, nullptr);
	}
	else if (tag == "route")
	{
		route(_lastIdx);
	}
}

void Axes::takeOldAxes(Axes *old)
{
	for (size_t i = 0; i < 3; i++)
	{
		_dirs[i] = old->_dirs[i];
		_targets[i] = old->_targets[i];
	}
	
	refreshAxes();
}

void Axes::interacted(int idx, bool hover, bool left)
{
	if (!hover && _instance && _cluster)
	{
		_lastIdx = idx;

		Menu *m = new Menu(_scene, this);
		m->addOption("explore axis", "explore_axis");
		m->addOption("reorient", "reorient");
		m->addOption("reflect", "reflect");
		
		if (_scene->colourRule() != nullptr)
		{
			m->addOption("match colour", "match_colour");
		}
		
		if (_targets[idx] != nullptr)
		{
			m->addOption("reset", "reset");
#ifdef VERSION_SHORT_ROUTES
			m->addOption("find route", "route");
#endif
		}
		
		double w = _scene->width();
		double h = _scene->height();
		int lx = -1; int ly = -1;
		_scene->getLastPos(lx, ly);
		double x = lx / w; double y = ly / h;
		m->setup(x, y);

		_scene->setModal(m);
	}
}

void Axes::reindex()
{
	size_t offset = indexOffset();

	for (size_t i = 0; i < _dirs.size(); i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			_vertices[i * 4 + j].extra[0] = i + 1 + offset;
		}
	}
}

size_t Axes::requestedIndices()
{
	return 3;
}

void Axes::refreshAxes()
{
	ObjectGroup *group = _cluster->objectGroup();
	int idx = group->indexOfObject(_instance);
	glm::vec3 start = glm::vec3(0.f);
	
	if (idx >= 0)
	{
		start = _cluster->pointForDisplay(idx);
	}

	for (size_t i = 0; i < 3; i++)
	{
		glm::vec3 dir = _dirs[i];
		_cluster->reweight(dir);
		
		glm::vec4 colour = glm::vec4(0., 0., 0., 0.);
		colour[2] = (_planes[i] ? 1. : 0.);

		for (size_t j = 0; j < 4; j++)
		{
			int index = i * 4 + j;
			_vertices[index].pos = start + (j % 2 == 1 ? dir : glm::vec3(0.f));
			_vertices[index].normal = dir;

			_vertices[index].color = colour;
		}
	}

	forceRender(true, false);
}

void Axes::prepareAxes()
{
	glm::vec3 centre = glm::vec3(0.f);
	
	if (_cluster && _instance)
	{
		ObjectGroup *group = _cluster->objectGroup();
		int idx = group->indexOfObject(_instance);
		centre = _cluster->pointForDisplay(idx);
	}

	for (size_t i = 0; i < _dirs.size(); i++)
	{
		glm::vec3 dir = _dirs[i];
		addThickLine(centre, dir);
	}
	
	refreshAxes();
}

void Axes::reflect(int i)
{
	_dirs[i] *= -1;
	refreshAxes();
}

void Axes::reorient(int i, Instance *mol)
{
	if (i >= 0 && i <= 2)
	{
		_targets[i] = mol;
	}

	if (i < 0 && _lastIdx >= 0)
	{
		_targets[_lastIdx] = mol;
	}
	else if (i < 0)
	{
		for (size_t j = 0; j < 3; j++)
		{
			_targets[j] = nullptr;
		}
	}

	ObjectGroup *group = _cluster->objectGroup();
	int idx = group->indexOfObject(_instance);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = 0; i < 3; i++)
	{
		if (_targets[i] != nullptr)
		{
			int tidx = group->indexOfObject(_targets[i]);
			glm::vec3 diff = _cluster->point(tidx) - centre;
			glm::vec3 norm = glm::normalize(diff);

			_dirs[i] = norm;
		}
		else
		{
			_dirs[i] = glm::vec3(0.f);
			_dirs[i][i] = 1.f;
		}
	}
	
	refreshAxes();
}

void Axes::backgroundPrioritise(std::string key)
{
	stop();
	_worker = new std::thread(&Axes::prioritiseDirection, this, key);
}

size_t Axes::parameterCount()
{
	return 3;
}

int Axes::sendJob(std::vector<float> &all)
{
	std::vector<float> vals = _cluster->objectGroup()->numbersForKey(_key);
	CorrelData cd = empty_CD();
	glm::vec3 dir = glm::normalize(glm::vec3(all[0], all[1], all[2]));

	for (int idx = 0; idx < _cluster->objectGroup()->objectCount(); idx++)
	{
		glm::vec3 centre = _cluster->point(idx);
		float pos = glm::dot(dir, centre);
		
		add_to_CD(&cd, vals[idx], pos);
	}
	
	int ticket = getNextTicket();
	float score = evaluate_CD(cd);

	_dirs[_lastIdx] = dir;
	
	if (score != score)
	{
		score = FLT_MAX;
	}

	refreshAxes();

	setScoreForTicket(ticket, score);
	return ticket;
}

void Axes::prioritiseDirection(std::string key)
{
	std::cout << "Prioritising direction against " << key << std::endl;
	_key = key;
	if (_engine != nullptr)
	{
		delete _engine;
		_engine = nullptr;
	}
	_engine = new ChemotaxisEngine(this);
	_engine->start();
	
	_key = "";
	std::string score = std::to_string(_engine->bestScore());

	_scene->setInformation("Correlation with colour: " + score);
}

