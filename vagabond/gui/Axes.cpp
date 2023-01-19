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
#include <vagabond/core/Molecule.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
#include "PlaneView.h"
#include "AxisExplorer.h"
#include "PlausibleRoute.h"
#include "SplitRoute.h"
#include "RouteExplorer.h"
#include "Axes.h"

Axes::Axes(TorsionCluster *group, Molecule *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_torsionCluster = group;
	_molecule = m;
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	if (m) { setSelectable(true); }
#endif
}

Axes::Axes(PositionalCluster *group, Molecule *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_positionalCluster = group;
	_molecule = m;
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	if (m) { setSelectable(true); }
#endif
}

Axes::Axes(RopeCluster *group, Molecule *m) : IndexResponder()
{
	initialise();

	_cluster = group;
	_molecule = m;
	
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

Axes::~Axes()
{
	delete _pv;
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
	if (_torsionCluster)
	{
		return std::vector<Angular>();
	}

	if (_targets[idx] != nullptr)
	{
		int mine = _torsionCluster->dataGroup()->indexOfObject(_molecule);
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
	
	std::string str = "Reference " + _molecule->id();
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
		AxisExplorer *ae = new AxisExplorer(_scene, _molecule, list, vals);
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
	SplitRoute *sr = new SplitRoute(_molecule, _torsionCluster, l);
	
	std::vector<Angular> values = directTorsionVector(idx);
	sr->setRawDestination(values);
	sr->setDestinationMolecule(_targets[idx]);

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
	else if (tag == "cancel_plane")
	{
		cancelPlane();
	}
	else if (tag == "start_plane")
	{
		setAxisInPlane(_lastIdx, true);
	} 
	else if (tag == "make_plane")
	{
		setAxisInPlane(_lastIdx, true);
		preparePlane();
	}
}

void Axes::takeOldAxes(Axes *old)
{
	PlaneView *pv = old->planeView();

	for (size_t i = 0; i < 3 && pv != nullptr; i++)
	{
		_planes[i] = pv->planes()[i];
	}
	
	for (size_t i = 0; i < 3; i++)
	{
		_dirs[i] = old->_dirs[i];
		_targets[i] = old->_targets[i];
	}
	
	refreshAxes();
	
	if (pv != nullptr)
	{
		preparePlane();
	}
}

void Axes::preparePlane()
{
	/*
	std::cout << "preparing plane" << std::endl;

	delete _pv; _pv = nullptr;
	_pv = new PlaneView(_cluster, _molecule);
	
	Plane *plane = _pv->plane();
	
	for (size_t i = 0; i < 3; i++)
	{
		if (!_planes[i])
		{
			continue;
		}

		std::vector<float> mapped = getMappedVector(i);
		
		std::vector<ResidueTorsion> list;
		list = _cluster->dataGroup()->headers();
		std::vector<Angular> vals = getTorsionVector(i);

		plane->addAxis(list, vals, mapped);
	}
	
	plane->setResponder(_pv);
	plane->refresh();
	_pv->setPlanes(_planes);
	_pv->populate();
	addObject(_pv);
	*/
}

void Axes::cancelPlane()
{
	for (size_t i = 0; i < 3; i++)
	{
		setAxisInPlane(i, false);
	}

	if (_pv != nullptr)
	{
		removeObject(_pv);
		delete _pv;
		_pv = nullptr;
	}
	
	refreshAxes();
}

void Axes::setAxisInPlane(int idx, bool plane)
{
	_planes[idx] = plane;
	refreshAxes();
}

bool Axes::startedPlane()
{
	for (size_t i = 0; i < 3; i++)
	{
		if (_planes[i])
		{
			return true;
		}
	}

	return false;
}

bool Axes::finishedPlane()
{
	int count = 0;

	for (size_t i = 0; i < 3; i++)
	{
		if (_planes[i])
		{
			count++;
		}
	}

	return (count >= 2);
}

void Axes::interacted(int idx, bool hover, bool left)
{
	if (!hover && _molecule && _cluster)
	{
		_lastIdx = idx;

		Menu *m = new Menu(_scene, this);
		m->addOption("explore axis", "explore_axis");
		m->addOption("reorient", "reorient");
		m->addOption("reflect", "reflect");
		
		if (_targets[idx] != nullptr)
		{
			m->addOption("reset", "reset");
#ifdef VERSION_SHORT_ROUTES
			m->addOption("find route", "route");
#endif
		}
		
#ifdef VERSION_NEXT
		if (!startedPlane())
		{
			m->addOption("start plane", "start_plane");
		}
		else if (!finishedPlane() && !_planes[_lastIdx])
		{
			m->addOption("make plane", "make_plane");
		}
		else if (!finishedPlane())
		{
			m->addOption("cancel plane", "cancel_plane");
		}
		else if (finishedPlane())
		{
			m->addOption("cancel plane", "cancel_plane");
			m->addOption("plane options", "plane_options");
		}
#endif

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
	int idx = group->indexOfObject(_molecule);
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

	rebufferVertexData();
}

void Axes::prepareAxes()
{
	glm::vec3 centre = glm::vec3(0.f);
	
	if (_cluster && _molecule)
	{
		ObjectGroup *group = _cluster->objectGroup();
		int idx = group->indexOfObject(_molecule);
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

void Axes::reorient(int i, Molecule *mol)
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
	int idx = group->indexOfObject(_molecule);
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
