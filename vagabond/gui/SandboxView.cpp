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
#include <vagabond/core/MappingToMatrix.h>
#include <vagabond/core/Network.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/PolymerEntityManager.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/SpecificNetwork.h>
#include <vagabond/core/PolymerEntity.h>
#include <vagabond/core/CompareDistances.h>

#include "SandboxView.h"

SandboxView::SandboxView(Scene *prev) : Display(prev)
{

}

SandboxView::~SandboxView()
{

}

void SandboxView::setup()
{

}

void SandboxView::makeMapping()
{
	Entity *e = &Environment::entityManager()->forPolymers()->object(0);
	std::vector<Instance *> insts = e->instances();
	
	Network *net = new Network(e, insts);
	_network = net;
	net->setup();

	_mapped = _network->blueprint();
	_specified = _network->specificForInstance(insts[0]);
	_specified->setResponder(this);
}

void SandboxView::makeTriangles()
{
	_mat2Map = new MappingToMatrix(*_mapped);

	std::cout << "Rastering..." << std::endl;
	MatrixPlot *mp = new MatrixPlot(_mat2Map->matrix(), _mutex);
	mp->resize(0.8);
	mp->setCentre(0.2, 0.5);
	mp->update();
	addObject(mp);
	_plot = mp;
}

