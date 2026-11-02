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

#include "AlignmentView.h"
#include "Mab.h"
#include "Mesh.h"
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/Instance.h>
#include <vagabond/gui/DisplayUnit.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/core/GroupBounds.h>
#include "GhostAb.h"
#include "ContactPoint.h"
#include <vagabond/gui/elements/TextButton.h>

AlignmentView::AlignmentView(Scene *prev, Antigen &antigen, 
                             Fiducial &fiducial, const glm::mat4x4 &transform,
                             const std::string &warning, bool poor)
: Scene(prev), Display(prev), _antigen(antigen), _fiducial(fiducial),
_transform(transform), _warning(warning), _poor(poor)
{
	_farSlab = 80;
	_slabbing = true;
	// for the antibody to glow
	setPingPong(true);
	shiftToCentre({}, 80);
}

void AlignmentView::setup()
{
	Display::setup();

	addTitle((_poor ? "Poor alignment: " : "Alignment: ") + _fiducial.name);

	Text *text = new Text((_poor ? "Warning: " : "") + _warning + ".\nThe antibody is shown "\
	                      "where this alignment puts it on the antigen mesh.");
	text->resize(0.6);
	text->setCentre(0.5, 0.17);
	addObject(text);

	// the mesh itself belongs to the antigen, so it is only borrowed
	_mesh = _antigen.mesh();
	addObject(_mesh);
	shiftToCentre(_mesh->centroid(), 0);

	// The antibody's model, put where the alignment says it goes (see
	// showFiducial()). Others of this antigen can be ticked on to compare.
	_shown.reserve(64);
	_shown.push_back({&_fiducial, _transform, nullptr, -1});
	if (_fiducial.contact)
	{
		const auto &all = _fiducial.contact->orientations();
		for (size_t i = 0; i < all.size(); i++)
		{
			if (all[i].transform == _transform)
			{
				_shown.back().orientation = i;
			}
		}
	}
	showFiducial(_shown.back());
	setupTickBoxes();
}

void AlignmentView::setupTickBoxes()
{
	if (!_fiducials)
	{
		return;
	}

	TickBoxes *tb = new TickBoxes(this, this);
	tb->setOneOnly(false);
	tb->setVertical(true);

	int count = 0;
	for (Fiducial &fid : *_fiducials)
	{
		if (fid.antigen != _fiducial.antigen || !fid.contact)
		{
			continue;
		}

		bool current = (&fid == &_fiducial);
		const std::vector<ContactPoint::Orientation> &orientations =
		fid.contact->orientations();
		if (!current && orientations.empty())
		{
			continue;
		}

		// the others are shown at their first orientation
		if (!current)
		{
			if (_shown.size() >= _shown.capacity())
			{
				break;
			}
			_shown.push_back({&fid, orientations.front().transform, nullptr, 0});
		}

		Shown *shown = current ? &_shown.front() : &_shown.back();
		std::string name = fid.name;
		tb->addOption(name, [this, tb, name, shown]()
		{
			if (tb->isTicked(name))
			{
				_active = shown - &_shown.front();
				showFiducial(*shown);
				updateOrientationLabel();
			}
			else
			{
				hideFiducial(*shown);
				if (_active == (size_t)(shown - &_shown.front()))
				{
					_active = 0;
				}
				updateOrientationLabel();
			}
		}, current);
		count++;
	}

	if (count > 1)
	{
		tb->arrange(0.02, 0.3, 0.25, 0.3 + 0.04 * count);
		addObject(tb);
	}
	else
	{
		delete tb;
	}

	// below the list: step the last ticked antibody through its orientations
	float y = 0.3 + 0.04 * count + 0.09;
	_orientLabel = new Text("");
	_orientLabel->resize(0.45);
	_orientLabel->setLeft(0.02, y);
	addObject(_orientLabel);

	TextButton *prev = new TextButton("<", this);
	prev->setLeft(0.02, y + 0.15);
	prev->setReturnJob([this]() { stepOrientation(-1); });
	addObject(prev);

	TextButton *next = new TextButton(">", this);
	next->setLeft(0.07, y + 0.15);
	next->setReturnJob([this]() { stepOrientation(1); });
	addObject(next);

	updateOrientationLabel();
}

void AlignmentView::updateOrientationLabel()
{
	if (!_orientLabel)
	{
		return;
	}

	Shown &shown = _shown[_active];
	size_t total = shown.fid->contact ? 
	shown.fid->contact->orientations().size() : 0;
	std::string text = shown.fid->name + "\n";
	if (shown.orientation < 0 || total == 0)
	{
		text += "orientation as handed in\n(" + std::to_string(total) + 
		" kept)";
	}
	else
	{
		float rmsd = shown.fid->contact->orientations()[shown.orientation].rmsd;
		text += "orientation " + std::to_string(shown.orientation + 1) + 
		" of " + std::to_string(total) + "\nRMSD " +
		(rmsd != rmsd ? std::string("unknown") : f_to_str(rmsd, 1) + " A");
	}

	_orientLabel->setText(text);
}

void AlignmentView::stepOrientation(int dir)
{
	Shown &shown = _shown[_active];
	if (!shown.unit || !shown.fid->contact)
	{
		return;
	}

	const auto &all = shown.fid->contact->orientations();
	int total = all.size();
	if (total < 2)
	{
		return;
	}

	int next = shown.orientation < 0 ? (dir > 0 ? 0 : total - 1) :
	(shown.orientation + dir + total) % total;
	shown.orientation = next;
	shown.transform = all[next].transform;

	// fresh from the file again, then put in its new place
	hideFiducial(shown);
	showFiducial(shown);
	updateOrientationLabel();
}

void AlignmentView::showFiducial(Shown &shown)
{
	if (shown.unit)
	{
		return;
	}

	Fiducial &fid = *shown.fid;
	fid.model.load();
	AtomGroup *atoms = fid.model.currentAtoms();

	// the antibody glows, so that it stands out from the antigen copies in
	// the same model (added colour; the glow needs the ping pong buffers)
	for (Instance *inst : fid.model.instances())
	{
		if (_antigen.entities.count(inst->entity_id()))
		{
			continue;
		}

		for (Atom *atom : inst->currentAtoms()->atomVector())
		{
			atom->setAddedColour(1.f);
		}
	}

	// It is shown first and moved afterwards, as the representations
	// (ribbons and so on) follow the atoms through the unit's watch, which
	// is only there once the unit has been displayed.
	shown.unit = new DisplayUnit(this);
	shown.unit->loadAtoms(atoms);
	shown.unit->displayAtoms();
	shown.unit->startWatch();
	addDisplayUnit(shown.unit);

	glm::mat4x4 transform = shown.transform;
	// displaying the atoms re-centres the view on the antibody where it
	// started, not where the alignment puts it, so the view is brought back
	// to the antigen once the antibody has been moved
	glm::vec3 centre = _mesh->centroid();
	Shown *which = &shown;
	GlowBall *ball = new GlowBall();
	addObject(ball);
	shown.midpoint = ball;
	addMainThreadJob([this, atoms, transform, centre, which, ball]()
	{
		for (Atom *atom : atoms->atomVector())
		{
			glm::vec3 d = atom->derivedPosition();
			atom->setDerivedPosition(glm::vec3(transform * glm::vec4(d, 1.f)));
		}

		shiftToCentre(centre, 0);

		// the midpoint is worked out as when mapping, for this orientation
		if (which->midpoint != ball)
		{
			return;
		}

		AtomGroup abs, anti;
		for (Instance *inst : which->fid->model.instances())
		{
			bool antigen = _antigen.entities.count(inst->entity_id());
			(antigen ? anti : abs).add(inst->currentAtoms());
		}

		glm::vec3 midpoint;
		if (ContactPoint::interfaceMidpoint(&abs, &anti, midpoint))
		{
			ball->show(midpoint);
		}
	});
}

void AlignmentView::hideFiducial(Shown &shown, bool destroying)
{
	if (!shown.unit)
	{
		return;
	}

	if (shown.midpoint)
	{
		removeObject(shown.midpoint);
		if (destroying)
		{
			delete shown.midpoint;
		}
		else
		{
			Window::setDelete(shown.midpoint);
		}
		shown.midpoint = nullptr;
	}

	removeDisplayUnit(shown.unit);
	shown.unit = nullptr;
	shown.fid->model.unload();
}

AlignmentView::~AlignmentView()
{
	if (_mesh)
	{
		removeObject(_mesh);
	}

	for (Shown &shown : _shown)
	{
		hideFiducial(shown, true);
	}
}
