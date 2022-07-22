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

#include <vagabond/core/MetadataGroup.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Menu.h>
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
	}
	
	prepareAxes();

#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
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

std::vector<float> Axes::getTorsionVector(int idx)
{
	int axis = _cluster->axis(idx);
	std::cout << "Axis: " << idx << " converts to " << axis << std::endl;
	if (_targets[idx] == nullptr)
	{
		std::vector<float> vals = _cluster->torsionVector(axis);
		return vals;
	}

	Molecule *other = _targets[idx];
	int which = _cluster->dataGroup()->indexOfObject(other);
	int mine = _cluster->dataGroup()->indexOfObject(_molecule);

	if (which < 0)
	{
		return std::vector<float>();
	}
	
	std::vector<float> vals;
	vals = _cluster->dataGroup()->differences(mine, which);
	return vals;
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
		str += " to target " + _targets[idx]->id();
		info = "Showing linear interpolation between torsion angles.\n"\
		"Rarely a realistic motion between conformational states.";
	}
	else
	{
		int axis = _cluster->axis(idx);
		str += " PCA axis " + i_to_str(axis);
		info = "Axis has been scaled to an average torsion angle\n"\
		"deviation of 4 degrees.";
	}

	try
	{
		AxisExplorer *ae = new AxisExplorer(_scene, _molecule, list, vals);
		ae->setFutureTitle(str);
		ae->show();
		ae->setInformation(info);
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
	else if (tag == "reset")
	{
		reorient(_lastIdx, nullptr);
	}
}

void Axes::interacted(int idx, bool hover, bool left)
{
	if (!hover)
	{
		_lastIdx = idx;

		Menu *m = new Menu(_scene, this);
		m->addOption("explore axis", "explore_axis");
		m->addOption("reorient", "reorient");
		
		if (_targets[idx] != nullptr)
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

		for (size_t j = 0; j < 4; j++)
		{
			_vertices[i * 4 + j].pos = centre + (j % 2 == 1 ? dir : glm::vec3(0.f));
			_vertices[i * 4 + j].normal = dir;
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
	
	refreshAxes();
}
