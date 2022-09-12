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
#include <vagabond/core/MetadataGroup.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
#include "PlaneView.h"
#include "AxisExplorer.h"
#include "Axes.h"

Axes::Axes(Cluster<MetadataGroup> *group, Molecule *m) : IndexResponder()
{
	setName("Axes");
	setImage("assets/images/axis.png");
	setUsesProjection(true);
	_cluster = group;
	_molecule = m;
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
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
}

Axes::~Axes()
{
	delete _pv;
	_molecule->model()->unload();
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


std::vector<float> Axes::getTorsionVector(int idx)
{
	if (_targets[idx] != nullptr)
	{
		int mine = _cluster->dataGroup()->indexOfObject(_molecule);
		int yours = _cluster->dataGroup()->indexOfObject(_targets[idx]);

		std::vector<float> from_vals, to_vals;
		from_vals = _cluster->dataGroup()->differenceVector(mine);
		to_vals = _cluster->dataGroup()->differenceVector(yours);

		for (size_t i = 0; i < to_vals.size(); i++)
		{
			to_vals[i] -= from_vals[i];
		}
		
		_cluster->dataGroup()->removeNormals(to_vals);
		return to_vals;
	}

	glm::vec3 dir = _dirs[idx];
	
	std::vector<float> sums;
	for (size_t i = 0; i < 3; i++)
	{
		if (i >= _cluster->rows())
		{
			continue;
		}
		int axis = _cluster->axis(i);
		std::vector<float> vals = _cluster->torsionVector(axis);

		float &weight = dir[i];
		
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
	std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();

	std::vector<float> vals = getTorsionVector(idx);
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
		int axis = _cluster->axis(idx);
		str += " PCA axis " + i_to_str(axis);
	}

	try
	{
		AxisExplorer *ae = new AxisExplorer(_scene, _molecule, list, vals);
		ae->setCluster(_cluster);
		ae->setFutureTitle(str);
		ae->show();
//		ae->setInformation(info);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << err.what() << std::endl;
	}

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
		
		std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();
		std::vector<float> vals = getTorsionVector(i);

		plane->addAxis(list, vals, mapped);
	}
	
	plane->setResponder(_pv);
	plane->refresh();
	_pv->setPlanes(_planes);
	_pv->populate();
	addObject(_pv);
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
	if (!hover)
	{
		_lastIdx = idx;

		Menu *m = new Menu(_scene, this);
		m->addOption("explore axis", "explore_axis");
		m->addOption("reorient", "reorient");
		m->addOption("reflect", "reflect");
		
		if (_targets[idx] != nullptr)
		{
			m->addOption("reset", "reset");
		}
		
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
	MetadataGroup *group = _cluster->dataGroup();
	int idx = group->indexOfObject(_molecule);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = 0; i < 3; i++)
	{
		glm::vec3 dir = _dirs[i];
//		dir *= _cluster->scaleFactor();
		
		/*
		if (_targets[i] != nullptr)
		{
			int tidx = group->indexOfObject(_targets[i]);
			
			if (tidx >= 0)
			{
				glm::vec3 diff = _cluster->point(tidx) - centre;
				glm::vec3 norm = glm::normalize(diff);

				dir = norm;
			}
		}
		*/

		for (size_t j = 0; j < 4; j++)
		{
			int index = i * 4 + j;
			_vertices[index].pos = centre + (j % 2 == 1 ? dir : glm::vec3(0.f));
			_vertices[index].normal = dir;
			
			_vertices[index].color = glm::vec4(0., 0., 0., 0.);
			_vertices[index].color[2] = (_planes[i] ? 1. : 0.);
		}
	}

	rebufferVertexData();
}

void Axes::prepareAxes()
{
	MetadataGroup *group = _cluster->dataGroup();
	int idx = group->indexOfObject(_molecule);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = 0; i < _dirs.size(); i++)
	{
		glm::vec3 dir = _dirs[i];

		{
			Snow::Vertex &v = addVertex(centre);
			v.tex[0] = -0.5;
			v.tex[1] = 0;
		}

		{
			Snow::Vertex &v = addVertex(centre + dir);
			v.tex[0] = -0.5;
			v.tex[1] = 1;
		}

		{
			Snow::Vertex &v = addVertex(centre);
			v.tex[0] = +0.5;
			v.tex[1] = 0;
		}

		{
			Snow::Vertex &v = addVertex(centre + dir);
			v.tex[0] = +0.5;
			v.tex[1] = 1;
		}

		addIndices(-4, -3, -2);
		addIndices(-3, -2, -1);
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

	MetadataGroup *group = _cluster->dataGroup();
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
