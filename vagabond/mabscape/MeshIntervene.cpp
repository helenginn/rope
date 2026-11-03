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

#include "MeshIntervene.h"
#include "Mab.h"
#include "Mesh.h"
#include <vagabond/utils/DoJob.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <cmath>

MeshIntervene::MeshIntervene(Scene *prev, Mab &mab,
                             const std::function<void()> &proceed)
: Scene(prev), Display(prev), _mab(mab), _proceed(proceed)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);
}

MeshIntervene::~MeshIntervene()
{
	if (_mesh)
	{
		removeObject(_mesh);
	}
}

bool MeshIntervene::settled()
{
	for (Antigen &antigen : _mab.antigens)
	{
		if (!antigen.mesh()->refined())
		{
			return false;
		}
	}
	return true;
}

void MeshIntervene::setup()
{
	Display::setup();
	addTitle("Mesh calculation");

	// placeholder: only the first antigen's mesh is on show
	if (_mab.antigens.size())
	{
		_mesh = _mab.antigens.front().mesh();
		addObject(_mesh);
		shiftToCentre(_mesh->centroid(), 0);
	}

	addBallSlider();
	addCurvatureTick();
	refreshButtons();
}

// the radius of the rolling ball of the grow-shrink cycle, from 0 (none) to 30
// Angstroms: a mesh already shrink wrapped has only broad shallow dents left,
// which take a big ball to span. Every change to the slider sets off a cycle, which is quick
void MeshIntervene::addBallSlider()
{
	const double min = 0; const double max = 30; const double step = 0.5;

	_ballText = new Text("");
	addObject(_ballText);

	Slider *s = new Slider();
	s->resize(0.5);
	s->setup("", min, max, step, false);
	s->setDragFunction([this](double val, double)
	{
		if (val == _ballRadius)
		{
			return;
		}

		_ballRadius = val;
		updateBallText();
		ballCycle();
	});
	s->setStart((_ballRadius - min) / (step * ((max - min) / step + 1)), 0.);
	s->setCentre(0.7, 0.9);
	addObject(s);

	updateBallText();
}

// colours the mesh by how curved it is at each vertex (Cluster4x: blue for
// concave to red for convex), which is what sets the radius of an antibody
// there. It follows the mesh as it changes.
void MeshIntervene::addCurvatureTick()
{
	const std::string text = "colour by curvature";
	TickBoxes *ticks = new TickBoxes(this, this);
	ticks->setOneOnly(false);
	ticks->addOption(text, [this, ticks, text]()
	{
		_colourCurvature = ticks->isTicked(text);
		colourMeshes();
	}, _colourCurvature);
	ticks->arrange(0.08, 0.78, 0.4, 0.86);
	addObject(ticks);
}

void MeshIntervene::colourMeshes()
{
	for (Antigen &antigen : _mab.antigens)
	{
		antigen.mesh()->setCurvatureColours(_colourCurvature);
	}
}

void MeshIntervene::updateBallText()
{
	char buf[64];
	snprintf(buf, sizeof(buf), "grow-shrink ball: %.2f A", (float)_ballRadius);
	_ballText->setText(buf);
	_ballText->resize(0.6);
	_ballText->setCentre(0.7, 0.84);
}

// A cycle on every mesh, unless something is already running: then it is
// left a note to go round again, with the latest radius, when it is done.
void MeshIntervene::ballCycle()
{
	if (_ballRadius < 0.05)
	{
		return;
	}

	if (_refining || _ballBusy.exchange(true))
	{
		_ballPending = true;
		return;
	}

	std::vector<Mesh *> meshes;
	for (Antigen &antigen : _mab.antigens)
	{
		meshes.push_back(antigen.mesh());
	}

	DoJob([this, meshes]()
	{
		do
		{
			_ballPending = false;
			float radius = _ballRadius;

			try
			{
				for (Mesh *mesh : meshes)
				{
					mesh->closeCycle(radius);
				}
			}
			catch (const std::exception &err)
			{
				std::cout << "Grow-shrink failed: " << err.what() << std::endl;
			}
		}
		while (_ballPending && !_refining);

		_ballBusy = false;
		addMainThreadJob([this]() { refreshButtons(); });
	});
}

void MeshIntervene::refreshButtons()
{
	deleteTemps();

	if (_refining)
	{
		Text *text = new Text("Working on mesh...");
		text->setCentre(0.5, 0.17);
		addTempObject(text);

		// the long jobs can be cut short, keeping what they have done
		if (_stoppable)
		{
			// well away from every other button, so that a click meant for
			// Stop can never land on one which appears when it is done
			TextButton *tb = new TextButton("Stop", this);
			tb->setCentre(0.5, 0.7);
			tb->setReturnJob([this]() { stop(); });
			addTempObject(tb);
		}
		return;
	}

	{
		TextButton *tb = new TextButton("Reset", this);
		tb->setLeft(0.1, 0.9);
		tb->setReturnJob([this]() { reset(); });
		addTempObject(tb);
	}

	{
		TextButton *tb = new TextButton("Even out", this);
		tb->setLeft(0.22, 0.9);
		tb->setReturnJob([this]() { evenOut(); });
		addTempObject(tb);
	}

	{
		TextButton *tb = new TextButton("Shrink wrap", this);
		tb->setLeft(0.36, 0.9);
		tb->setReturnJob([this]() { shrinkWrap(); });
		addTempObject(tb);
	}

	if (settled())
	{
		TextButton *tb = new TextButton("Next", this);
		tb->setRight(0.9, 0.9);
		tb->setReturnJob(_proceed);
		addTempObject(tb);
	}
}

void MeshIntervene::reset()
{
	// the calculation takes a while: as for the others, it is for a worker
	// thread, not for the one which draws
	work([](Mesh *mesh) { mesh->reset(); }, false);
}

void MeshIntervene::evenOut()
{
	work([](Mesh *mesh) { mesh->evenOut(); });
}

void MeshIntervene::stop()
{
	_stopRequested = true;
	for (Antigen &antigen : _mab.antigens)
	{
		antigen.mesh()->requestStop();
	}
}

void MeshIntervene::shrinkWrap()
{
	work([](Mesh *mesh) { mesh->shrinkWrap(0.f); });
}

void MeshIntervene::work(const std::function<void(Mesh *)> &job,
                         bool stoppable)
{
	if (_refining || _ballBusy)
	{
		return;
	}

	_refining = true;
	_stoppable = stoppable;
	_stopRequested = false;
	refreshButtons();

	// the meshes are made here, not in the worker (which loads no models)
	std::vector<Mesh *> meshes;
	for (Antigen &antigen : _mab.antigens)
	{
		meshes.push_back(antigen.mesh());
		meshes.back()->clearStop();
	}

	DoJob([this, meshes, job]()
	{
		try
		{
			for (Mesh *mesh : meshes)
			{
				if (_stopRequested)
				{
					break;
				}
				job(mesh);
			}
		}
		catch (const std::exception &err)
		{
			std::cout << "Mesh work failed: " << err.what() << std::endl;
		}

		addMainThreadJob([this]()
		{
			_refining = false;
			_stoppable = false;
			refreshButtons();
		});
	});
}
