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

#include <iostream>
#include <fstream>
#include <thread>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/utils/MappingToMatrix.h>

#include "SandboxView.h"

SandboxView::SandboxView(Scene *prev) : Scene(prev)
{

}

SandboxView::~SandboxView()
{

}

void SandboxView::setup()
{
	makeMapping();
	makeTriangles();
	addTitle("Triangle mayhem");
}

void SandboxView::makeMapping()
{
	Face<0, 2, float> *p1 = new Face<0, 2, float>({0.f, -0.1f}, 0.2);
	Face<0, 2, float> *p2 = new Face<0, 2, float>({0.f, 1.f}, 0.8);
	Face<0, 2, float> *p3 = new Face<0, 2, float>({1.0f, 0.0f}, 1.8);
	Face<0, 2, float> *p4 = new Face<0, 2, float>({0.75f, 0.78f}, -0.5);
	Face<0, 2, float> *p5 = new Face<0, 2, float>({2.0f, 0.5}, -0.5);

	_mapped.addTriangle(p1, p2, p3);
	_mapped.addTriangle(p2, p3, p4);
	_mapped.addTriangle(p3, p4, p5);

}

void SandboxView::makeTriangles()
{
	_mat2Map = new MappingToMatrix(_mapped);
	MatrixPlot *mp = new MatrixPlot(_mat2Map->matrix(), _mutex);
	mp->update();
	addObject(mp);
}
