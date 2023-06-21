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
#include <vagabond/gui/ShowMap.h>
#include <vagabond/gui/GuiAtom.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/MappingToMatrix.h>
#include <vagabond/core/SpecificNetwork.h>
#include <vagabond/core/Cartographer.h>
#include <vagabond/core/PolymerEntity.h>
#include <vagabond/core/CompareDistances.h>
#include <vagabond/gui/elements/TextButton.h>

#include "MapView.h"

MapView::MapView(Scene *prev, Entity *entity, std::vector<Instance *> instances) 
: Display(prev)
{
	_cartographer = new Cartographer(entity, instances);
	_cartographer->setResponder(this);
	setOwnsAtoms(false);
}

MapView::~MapView()
{
	if (_worker)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}
}

void MapView::setup()
{
	_cartographer->setup();
	makeTriangles();
	addTitle("Triangle mayhem");

	_specified = _cartographer->specified();
	Instance *inst = _specified->instance();
	inst->currentAtoms()->recalculate();
	loadAtoms(inst->currentAtoms());

	VisualPreferences *vp = &inst->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp);

	_worker = new std::thread(Cartographer::assess, _cartographer);

	TextButton *command = new TextButton("Save model", this);
	command->setCentre(0.9, 0.9);
	command->setReturnTag("save_model");
	addObject(command);
}

void MapView::showMap()
{
	ShowMap *show = new ShowMap(this, _specified);
	show->setCartographer(_cartographer);
	show->setMapView(this);
	show->show();
}

void MapView::addButtons()
{
	if (_command)
	{
		removeObject(_command);
		delete _command;
		removeObject(_second);
		delete _second;
		removeObject(_showMap);
		delete _showMap;
	}

	{
		TextButton *command = new TextButton("Flip torsions", this);
		command->setCentre(0.35, 0.9);
		command->setReturnTag("flip");
		addObject(command);
		_command = command;
	}

	{
		TextButton *command = new TextButton("Nudge", this);
		command->setCentre(0.65, 0.9);
		command->setReturnTag("nudge");
		addObject(command);
		_second = command;
	}

	{
		TextButton *command = new TextButton("Examine", this);
		command->setCentre(0.15, 0.75);
		command->setReturnTag("show_map");
		addObject(command);
		_showMap = command;
	}
}

void MapView::stopWorker()
{
	_worker->detach();
	_command->setReturnTag("none");
	_command->setInert(true);
	_cartographer->stopASAP();
}

void MapView::skipJob()
{
	_cartographer->skipCurrentJob();
}

void MapView::cleanupPause()
{
	_refined = true;
	_specified->setResponder(this);

	if (_worker)
	{
		delete _worker;
		_worker = nullptr;
	}

	_command->setInert(false);
	_command->setText("Flip torsions");
	_command->setReturnTag("flip");

	_second->setInert(false);
	_second->setText("Nudge");
	_second->setReturnTag("nudge");

	_showMap->setText("Examine");
	_showMap->setReturnTag("show_map");
}

void MapView::makePausable()
{
	_specified->setDisplayInterval(100);
	_specified->removeResponder(this);
	_refined = false;

	_command->setText("Stop");
	_command->setReturnTag("pause");

	_second->setText("Skip");
	_second->setReturnTag("skip");

	_showMap->setText(" ");
	_showMap->setReturnTag("none");
}

void MapView::askForNudgePoint()
{
	setInformation("Click on point to nudge");
	_waitingForNudge = true;
}

void MapView::startNudges(std::vector<float> point)
{
	_cartographer->map2Matrix()->fraction_to_cart(point);
	makePausable();
	_worker = new std::thread(Cartographer::nudge, _cartographer, point);
}

void MapView::startFlip(int i, int j)
{
	makePausable();
	_worker = new std::thread(Cartographer::flipIdx, _cartographer, i, j);
}

void MapView::startFlips()
{
	makePausable();
	_worker = new std::thread(Cartographer::flip, _cartographer);
}

void MapView::makeTriangles()
{
	MatrixPlot *mp = new MatrixPlot(_cartographer->matrix(), _mutex);
	mp->setCentre(0.15, 0.5);
	mp->resize(0.8);
	mp->update();
	addObject(mp);
	_plot = mp;
}

bool MapView::plotPosition(float x, float y)
{
	if ((x < 0 || x > 1) || (y < 0 || y > 1))
	{
		return false;
	}

	std::vector<float> vals = {x, y};
	_cartographer->map2Matrix()->fraction_to_cart(vals);
	int num = _specified->submitJob(true, vals);
	_specified->retrieve();
	float score = _specified->deviation(num);
	std::string str = std::to_string(score);
	bool valid = _specified->valid_position(vals);

	if (!valid)
	{
		str += " - invalid";
	}

	setInformation(str);
	return true;
}

bool MapView::sampleFromPlot(double x, double y)
{
	if (!_refined)
	{
		return false;
	}

	double tx = x;
	double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;
	
	if (!_waitingForNudge)
	{
		return plotPosition(v.x, v.y);
	}
	else
	{
		_waitingForNudge = false;
		startNudges({v.x, v.y});
		return true;
	}

	return false;
}

void MapView::mouseMoveEvent(double x, double y)
{
	if (_left)
	{
		bool success = sampleFromPlot(x, y);
		
		if (success)
		{
			_editing = true;
			return;
		}
	}

	if (!_editing)
	{
		Display::mouseMoveEvent(x, y);
	}
}

void MapView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "flip")
	{
		startFlips();
	}

	if (tag == "show_map")
	{
		showMap();
	}

	if (tag == "save_model")
	{
		_atoms->writeToFile("tmp.pdb");
	}

	if (tag == "nudge")
	{
		askForNudgePoint();
	}

	if (tag == "pause")
	{
		stopWorker();
	}

	if (tag == "skip")
	{
		skipJob();
	}

	Display::buttonPressed(tag, button);
}

void MapView::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	sampleFromPlot(x, y);
	_editing = false;
	
	Display::mousePressEvent(x, y, button);
}

void MapView::displayDistances(PCA::Matrix &dist)
{
	_distMat = dist;

	if (_distances == nullptr)
	{
		_distances = new MatrixPlot(_distMat, _mutex);
		_distances->resize(0.8);
		_distances->setCentre(0.85, 0.5);
		addObject(_distances);
	}

	_distances->update();
}

void MapView::sendObject(std::string tag, void *object)
{
	if (tag == "update_map")
	{
		_updatePlot = true;
	}

	if (tag == "update_score")
	{
		float current = *static_cast<float *>(object);
		setInformation(f_to_str(current, 3));
	}

	if (tag == "atom_matrix")
	{
		PCA::Matrix *dist = static_cast<PCA::Matrix *>(object);
		displayDistances(*dist);
	}

	if (tag == "atom_map")
	{
		AtomPosMap *aps = static_cast<AtomPosMap *>(object);
		CompareDistances cd;
		cd.process(*aps);
		PCA::Matrix dist = cd.matrix();
		displayDistances(dist);
	}

	else if (tag == "done")
	{
		_refined = true;
		_specified->setDisplayInterval(1);
		_specified->setResponder(this);
		_updateButtons = true;
	}

	else if (tag == "paused")
	{
		_cleanupPause = true;
	}

}

void MapView::doThings()
{
	if (_updatePlot)
	{
		_plot->update();
		_updatePlot = false;
	}

	if (_cleanupPause)
	{
		cleanupPause();
		_cleanupPause = false;
	}

	if (_updateButtons)
	{
		addButtons();
		_updateButtons = false;
	}
}
