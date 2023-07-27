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
#include <vagabond/core/Warp.h>
#include <vagabond/core/AtomWarp.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>

#include "MapView.h"

MapView::MapView(Scene *prev, Entity *entity, std::vector<Instance *> instances,
                 Instance *reference) 
: Display(prev)
{
	_reference = reference;
	_instances = instances;
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
	showWarpSpace();
	addTitle("Warp space");

	_reference->load();
	_reference->currentAtoms()->recalculate();
	loadAtoms(_reference->currentAtoms());
	
	_warp = new Warp(_reference, 2);
	_warp->setup();
	
	AtomWarp aw(_instances, _reference);
	std::function<Vec3s(const Coord::Get &)> motions;
	motions = aw.mappedMotions(2, _warp->atomList());
	_warp->setAtomMotions(motions);

	VisualPreferences *vp = &_reference->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp);
	
	addButtons();

//	_worker = new std::thread(Cartographer::assess, _cartographer);

	TextButton *command = new TextButton("Save space", this);
	command->setCentre(0.9, 0.1);
	command->setReturnTag("save_space");
	addObject(command);
}

void MapView::addButtons()
{
	if (_toggle)
	{
		removeObject(_toggle);
		delete _toggle;
	}

	{
		TextButton *command = new TextButton("Displaying targets", this);
		command->setCentre(0.35, 0.9);
		command->setReturnTag("toggle");
		addObject(command);
		_toggle = command;
	}

	/*
	{
		TextButton *command = new TextButton("Nudge", this);
		command->setCentre(0.65, 0.9);
		command->setReturnTag("nudge");
		addObject(command);
		_second = command;
	}
	*/

	/*
	Text *info = new Text("C-alpha trajectory off-linear course plot");
	info->resize(0.4);
	info->setCentre(0.85, 0.22);
	addObject(info);
	*/
}

void MapView::stopWorker()
{
	_worker->detach();
	_toggle->setReturnTag("none");
	_toggle->setInert(true);
}

void MapView::skipJob()
{
	
}

void MapView::cleanupPause()
{
	_refined = true;

	if (_worker)
	{
		delete _worker;
		_worker = nullptr;
	}

	_second->setInert(false);
	_second->setText("Nudge");
	_second->setReturnTag("nudge");
}

void MapView::makePausable()
{
	_refined = false;

	_toggle->setText("Stop");
	_toggle->setReturnTag("pause");

	_second->setText("Skip");
	_second->setReturnTag("skip");
}

void MapView::showWarpSpace()
{
	setupMatrix(&_warpMat, _divisions, _divisions);
	MatrixPlot *mp = new MatrixPlot(_warpMat, _mutex);
	mp->setCentre(0.15, 0.5);
	mp->resize(0.8);
	mp->update();
	addObject(mp);
	_plot = mp;
}

bool MapView::plotPosition(float x, float y)
{
	_last = {x, y};
	std::vector<float> vals = {2*x - 1, 2*y - 1};
	int num = _warp->submitJob(true, vals);
	_warp->retrieve();
	float score = _warp->deviation(num);
	std::string str = std::to_string(score);
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

	if ((v.x < 0 || v.x > 1) || (v.y < 0 || v.y > 1))
	{
		return false;
	}

	return plotPosition(v.x, v.y);
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

void MapView::saveSpace(std::string filename)
{
	if (filename == "rope.json")
	{
		BadChoice *bc = new BadChoice(this, "rope.json is a reserved filename");
		setModal(bc);
		return;
	}

	json j{}; // = thing;

	std::ofstream file;
	file.open(filename);
	file << j.dump(1);
	file << std::endl;
	file.close();
}

void MapView::askForFilename()
{
	AskForText *aft = new AskForText(this, "Enter json filename to save", 
	                                 "json_filename", this);
	setModal(aft);
}

void MapView::toggleAtomDisplayType()
{
	bool set = _warp->isDisplayingTargets();
	_warp->setDisplayingTargets(!set);

	std::string str = !set ? "Displaying targets" : "Displaying predicted";
	_toggle->setText(str);
	plotPosition(_last[0], _last[1]);
}

void MapView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "toggle")
	{
		toggleAtomDisplayType();

	}
	if (tag == "json_filename")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string text = te->scratch();
		saveSpace(text);
	}

	if (tag == "save_space")
	{
		askForFilename();
	}

	if (tag == "save_model")
	{
		_atoms->writeToFile("tmp.pdb");
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
		setInformation(f_to_str(current, 5));
	}

	if (tag == "atom_matrix")
	{
		PCA::Matrix *dist = static_cast<PCA::Matrix *>(object);
		displayDistances(*dist);
	}

	if (tag == "atom_list")
	{
		AtomPosList *apl = static_cast<AtomPosList *>(object);
		CompareDistances cd;
		cd.process(*apl);
		PCA::Matrix dist = cd.matrix();
		displayDistances(dist);
	}

	else if (tag == "done")
	{
		_refined = true;
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
