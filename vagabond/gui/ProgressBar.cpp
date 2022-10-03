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
#include <vagabond/core/Environment.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/elements/SnowGL.h>

ProgressBar::ProgressBar(std::string text) : Image("assets/images/rope.png")
{
	resize(0.25);
	setFragmentShaderFile("assets/shaders/progress.fsh");
	
	if (text == "")
	{
		text = "Loading...";
	}
	Text *t = new Text(text);
	t->resize(0.4);
	t->setCentre(0.0, -0.03);
	addObject(t);

	setCentre(0.5, 0.88);
	Environment::env().setProgressResponder(this);
}

ProgressBar::~ProgressBar()
{
	Environment::env().setProgressResponder(nullptr);
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
}

void ProgressBar::sendObject(std::string tag, void *object)
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
	checkErrors("rebinding fraction");
}

