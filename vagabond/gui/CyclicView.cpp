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

#include "CyclicView.h"
#include <iostream>
#include <fstream>
#include <vagabond/gui/elements/Icosahedron.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/FileManager.h>

CyclicView::CyclicView(Cyclic *cyclic)
{
	setName("CyclicView");
	_renderType = GL_LINES;
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
	
	if (cyclic)
	{
		_cyclic = cyclic;
	}
	else
	{
		_cyclic = new Cyclic();
		_cyclic->addNextAtomName("N", 1.468,  109.05);
		_cyclic->addNextAtomName("CA", 1.534, 103.43);
		_cyclic->addNextAtomName("CB", 1.508, 103.51);
		_cyclic->addNextAtomName("CG", 1.515, 104.50);
		_cyclic->addNextAtomName("CD", 1.468, 105.70);
		_cyclic->setResponder(this);

		/*
		_cyclic->addNextAtomName("N", 1.468,  111.7);
		_cyclic->addNextAtomName("CA", 1.531, 103.3);
		_cyclic->addNextAtomName("CB", 1.495, 104.8);
		_cyclic->addNextAtomName("CG", 1.502, 106.5);
		_cyclic->addNextAtomName("CD", 1.474, 103.2);
		*/
	}
	
	populate();
	return;
	
	/*
	std::string proline = "assets/geometry/proline.json";
#ifndef __EMSCRIPTEN__
	if (!file_exists(proline))
	{
		FileManager::correctFilename(proline);
	}
#endif

	if (!file_exists(proline))
	{
		_cyclic->addNextAtomName("N", 1.468,  111.7);
		_cyclic->addNextAtomName("CA", 1.531, 103.3);
		_cyclic->addNextAtomName("CB", 1.495, 104.8);
		_cyclic->addNextAtomName("CG", 1.502, 106.5);
		_cyclic->addNextAtomName("CD", 1.474, 103.2);
		_cyclic->setResponder(this);
	}
	else
	{
		json data;
		std::ifstream f;
		f.open(proline);
		f >> data;
		f.close();
		
		Cyclic *c = new Cyclic(data["cyclic"]);
		_cyclic = c;
	}

	populate();
	*/
}

CyclicView::~CyclicView()
{

}

void CyclicView::updateCurve()
{
	_cyclic->updateCurve();

	for (float d = 0; d < _cyclic->pointCount(); d++)
	{
		const glm::vec3 &v = _cyclic->point(d);
		_vertices[d].pos = v;
	}
	
	forceRender(true);
}

void CyclicView::addCurve()
{
	addIndex(0);
	for (float d = 0; d < _cyclic->pointCount(); d++)
	{
		glm::vec3 v = glm::vec3(0.f);
		addVertex(v);
		addIndex(-1);
		addIndex(-1);
	}
	
	setAlpha(1.f);
}

void CyclicView::updateBalls()
{
	for (size_t i = 0; i < _cyclic->atomCount(); i++)
	{
		const glm::vec3 &p = _cyclic->atomPos(i);
		Icosahedron *ico = _balls[i];
		ico->setPosition(p);
		ico->forceRender(true, false);
	}
}

void CyclicView::addBalls()
{
	for (size_t i = 0; i < _cyclic->atomCount(); i++)
	{
		Icosahedron *ico = new Icosahedron();
		ico->triangulate();
		ico->resize(0.1);
		ico->setColour(0.5, 0.5, 0.5);
		if (i == 0)
		{
			ico->setColour(0.2, 0.2, 0.8);
		}
		if (i == 3)
		{
			ico->setColour(0.1, 0.1, 0.1);
		}
		ico->setAlpha(1.f);
		addObject(ico);
		_balls.push_back(ico);
	}
}

void CyclicView::populate()
{
	_cyclic->setup();
	addCurve();
	addBalls();
	updateBalls();
	updateCurve();
}

void CyclicView::respond()
{
	updateCurve();
	updateBalls();
	triggerResponse();
}

void CyclicView::increment()
{
	if (_rotate)
	{
		respond();
	}
}

std::string CyclicView::angles()
{
	return _cyclic->angles();
}

std::string CyclicView::lengths()
{
	return _cyclic->lengths();
}

float CyclicView::score()
{	
	float lsc = _cyclic->score();
	return lsc;
}

void CyclicView::refine()
{
	json data;
	data["cyclic"] = *_cyclic;
	
	std::ofstream file;
	file.open("proline.json");
	file << data;
	file << std::endl;
	file.close();

	_cyclic->refine();
}
