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
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/maths.h>
#include <vagabond/core/ChemotaxisEngine.h>
#include <vagabond/core/ObjectGroup.h>
#include <vagabond/core/HasMetadata.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/BadChoice.h>
#include "VagWindow.h"
#include "ConfSpaceView.h"
#include "PlausibleRoute.h"
#include "RouteExplorer.h"
#include "Axes.h"

Axes::Axes(ObjectGroup *data, ClusterSVD *cluster, HasMetadata *m) 
: IndexResponder()
{
	initialise();

	_data = data;
	_cluster = cluster;
	_focus = m;
	
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

void Axes::indicesOfObjectsPointedAt(int &start, int &end, int idx)
{
	if (idx < 0) idx = _lastIdx;
	
	if (_targets[idx] == nullptr)
	{
		return;
	}
	
	start = _data->indexOfObject(_focus);
	end = _data->indexOfObject(_targets[idx]);
}

std::string Axes::titleForAxis(int idx)
{
	if (idx < 0) idx = _lastIdx;
	
	if (_lastIdx < 0)
	{
		return "";
	}

	std::string str = "Reference " + _focus->id();
	std::string info;
	
	if (_targets[idx] != nullptr)
	{
		str += " axis to target " + _targets[idx]->id();
	}
	else
	{
		int axis = _cluster->axis(idx);
		str += " PCA axis " + i_to_str(axis);
	}

	return str;
}

void Axes::buttonPressed(std::string tag, Button *button)
{
	if (tag == "reorient")
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
	else
	{
		_data->doRequest(this, tag);
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

void Axes::assembleMenu()
{
	Menu *m = new Menu(_scene, this);

	_data->editMenu(this, m);

	m->addOption("reorient", "reorient");
	m->addOption("reflect", "reflect");

	if (_scene->colourRule() != nullptr)
	{
		m->addOption("match colour", "match_colour");
	}
	
	if (_edited[_lastIdx])
	{
		m->addOption("reset", "reset");
	}

	double w = _scene->width();
	double h = _scene->height();
	int lx = -1; int ly = -1;
	_scene->getLastPos(lx, ly);
	double x = lx / w; double y = ly / h;
	m->setup(x, y);

	_scene->setModal(m);
}

void Axes::interacted(int idx, bool hover, bool left)
{
	if (!hover && _focus && _cluster)
	{
		_lastIdx = idx;
		assembleMenu();
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
	int idx = _data->indexOfObject(_focus);
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
	
	if (_cluster && _focus)
	{
		int idx = _data->indexOfObject(_focus);
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

void Axes::reorient(int i, HasMetadata *mol)
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

	int idx = _data->indexOfObject(_focus);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = 0; i < 3; i++)
	{
		if (_targets[i] != nullptr)
		{
			int tidx = _data->indexOfObject(_targets[i]);
			glm::vec3 diff = _cluster->point(tidx) - centre;
			glm::vec3 norm = glm::normalize(diff);

			_dirs[i] = norm;
			_edited[i] = true;
		}
		else
		{
			_dirs[i] = glm::vec3(0.f);
			_dirs[i][i] = 1.f;
			_edited[i] = false;
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

int Axes::sendJob(const std::vector<float> &all)
{
	std::vector<float> vals = _data->numbersForKey(_key);
	CorrelData cd = empty_CD();
	glm::vec3 dir = glm::normalize(glm::vec3(all[0], all[1], all[2]));

	for (int idx = 0; idx < _data->objectCount(); idx++)
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
	
	if (_lastIdx >= 0)
	{
		_edited[_lastIdx] = true;
	}
	
	_key = "";
	std::string score = std::to_string(_engine->bestScore());

	_scene->setInformation("Correlation with colour: " + score);
}

