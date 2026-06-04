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

#include "ContactPoint.h"
#include "Playground.h"
#include "Positions.h"
#include "PosToComp.h"
#include "AbWatch.h"
#include "Mesh.h"
#include "Mab.h"
#include <vagabond/gui/elements/Icosahedron.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/MatrixPlot.h>

Playground::Playground(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab)
{
	_farSlab = 80;
	_slabbing = true;
	setPingPong(true);
	shiftToCentre({}, 80);

}

void Playground::showMesh(const Competition &comp)
{
	std::string antigen_name = comp.antigen;
	Antigen *antigen = _mab.antigens(antigen_name);

	Mesh *mesh = antigen->mesh();
	addObject(mesh);
	shiftToCentre(mesh->centroid(), 0);
	std::cout << "Mesh centre: " << mesh->centroid() << std::endl;
	_mesh = mesh;

}

void Playground::showFiducials(const Competition &comp)
{
	_positions = new Positions(*_mab.antigens(comp.antigen), comp, _mab,
	                           _mesh->vertexFinder(), _mesh->random());
	_watches = {};
	_positions->loadAntibodiesInto(this, _watches);
	
	for (AbWatch *aw : _watches)
	{
		addIndexResponder(aw);
	}
}

void Playground::setup()
{
	Competition &comp = *_mab.competitions.begin();
	showMesh(comp);
	showFiducials(comp);
	
	TextButton *tb = new TextButton("Menu");
	tb->setCentre(0.9, 0.1);

	auto show_menu = [this, tb]()
	{
		Menu *menu = new Menu(this);
		menu->addOption("show matrices", [this]() { showMatrices(); });
		menu->addOption("refine", [this]() { refine(); });
		menu->setup(tb, 1);
		setModal(menu);
	};
	
	tb->setReturnJob(show_menu);
	addObject(tb);
	
	Display::setup();
}

void Playground::interactedWithNothing(bool left, bool hover)
{
	for (AbWatch *aw : _watches)
	{
		aw->setHighlighted(false);
		aw->deleteTemps();
	}
}

void Playground::showMatrices()
{
	Competition &comp = *_mab.competitions.begin();
	if (!_model)
	{
		_model = new PosToComp(comp, *_positions);
	}

	if (!_mpModel)
	{
		std::mutex *mutex{};
		PCA::Matrix &display = _model->modelDisplay(&mutex);
	
		MatrixPlot *mp = new MatrixPlot(display, *mutex);
		Eigen::MatrixXf mat = _model->fromModel();

		mp->resize(0.4);
		mp->setCentre(0.9, 0.85);
		addObject(mp);
		_mpModel = mp;
	}

	if (!_mpData)
	{
		std::mutex *mutex{};
		PCA::Matrix &display = _model->dataDisplay(&mutex);

		MatrixPlot *mp = new MatrixPlot(display);
		mp->resize(0.4);
		mp->setCentre(0.75, 0.85);
		addObject(mp);
		_mpData = mp;
	}
	
	_model->setPlot(_mpModel);
}

void Playground::refine()
{
	showMatrices();
	_model->refine();
}
