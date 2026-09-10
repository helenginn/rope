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

#include "ProgressBar.h"
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/gui/elements/GLView.h>

ProgressBar::ProgressBar(std::string text) : Image("assets/images/rope.png")
{
	resize(0.25);
	setFragmentShaderFile("assets/shaders/progress.fsh");
	
	if (text == "")
	{
		text = "Loading...";
	}
	Text *t = new Text(text, Font::Thin, true);
	t->resize(0.4);
	t->setCentre(0.0, -0.03);
	addObject(t);

	setCentre(0.5, 0.88);
	setName("Progress bar");
}

ProgressBar::~ProgressBar()
{

}

void ProgressBar::setCancelJob(const std::function<void()> &job)
{
	if (_cancelButton)
	{
		removeObject(_cancelButton);
		Window::setDelete(_cancelButton);
		_cancelButton = nullptr;
	}

	if (!job)
	{
		return;
	}

	ImageButton *tb = new ImageButton("assets/images/cross.png", nullptr);
	tb->resize(0.06);
	tb->setCentre(0.6, 0.88);
	tb->setReturnJob(job);
	addObject(tb);
	_cancelButton = tb;
}

void ProgressBar::setSkipJob(const std::function<void()> &job)
{
	if (_skipButton)
	{
		removeObject(_skipButton);
		Window::setDelete(_skipButton);
		_skipButton = nullptr;
	}

	if (!job)
	{
		return;
	}

	// same shared arrow.png as ImageButton::arrow(), rotated -90 (i.e.
	// right-facing, per that helper's own convention) but sized/placed to
	// match the cross button rather than arrow()'s fixed rescale(0.1,
	// 0.03), so the two sit next to each other at the same scale.
	//
	// non-uniform on purpose: arrow.png is a tall down-pointing arrow
	// (portrait), and a -90 rotation swaps its axes, so its *height*
	// (the long, pointed shaft) becomes the button's on-screen width -
	// scaling that axis down to a third keeps the arrowhead
	// proportioned like a "next" glyph instead of a long pointer.
	ImageButton *tb = new ImageButton("assets/images/arrow.png", nullptr);
	tb->rescale(0.06, 0.02);
	glm::mat3x3 rot;
	rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(-90.),
	                              glm::vec3(0., 0., -1.)));
	tb->rotateRoundCentre(rot);

	// anchored by its left edge, not its centre: setCentre(0.68, ...) put
	// the old, 3x-wider arrow's centre here, which - now that it's a
	// third as wide - left a gap between it and the cross button instead
	// of sitting right next to it. Left-anchoring at the old arrow's own
	// left edge keeps this spot fixed regardless of how wide the glyph
	// itself is.
	tb->setLeft(0.63, 0.88);
	tb->setReturnJob(job);
	tb->addAltTag("Skip current subnetwork");
	addObject(tb);
	_skipButton = tb;
}

void ProgressBar::setMaxTicks(int count)
{
	_maxTicks = count;
	
	if (_maxTicks <= 0)
	{
		finish();
	}
}

void ProgressBar::finish()
{
	setDisabled(true);
	VagWindow::window()->requestProgressBarRemoval();
}

void ProgressBar::sendObject(std::string tag, void *object)
{
	// Progressor::clickTicker()/finishTicker() (Progressor.h) call this
	// synchronously, on whatever thread the underlying job runs on -
	// often a worker thread (e.g. ModelManager::autoModel()/rescan(),
	// called from Dictator's worker thread). finish() below touches
	// renderable/GL state (setDisabled(), _gl->viewChanged()), which
	// must only happen on the main thread - defer the actual handling.
	//
	// Deferred via VagWindow's own job queue, not this object's own: as a
	// window-level object (added via VagWindow::addObject(), not part of
	// the current Scene's tree), nothing ever calls this object's own
	// doJobs() to drain it - only curr->doThingsCircuit() (Window.cpp,
	// called on the current Scene) recurses doJobs() into Scene children.
	// Window-level objects only ever get ->render() called on them
	// (Window::render()). Queuing here instead would silently never run.
	VagWindow::window()->addMainThreadJob([this, tag]()
	                 {
		                handleProgressEvent(tag);
	});
}

void ProgressBar::handleProgressEvent(std::string tag)
{
	if (tag == "tick")
	{
		_ticks++;
		if (_ticks >= _maxTicks)
		{
			finish();
		}

		if (_gl)
		{
			_gl->viewChanged();
		}
	}
	else if (tag == "done")
	{
		finish();
	}
}

void ProgressBar::extraUniforms()
{
	float fraction = (float)_ticks / (float)_maxTicks;
	const char *uniform_name = "fraction";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, fraction);
}

