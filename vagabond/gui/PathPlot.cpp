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

#include "PathPlot.h"
#include <vagabond/gui/elements/Window.h>
#include <vagabond/core/paths/Monitor.h>

PathPlot::PathPlot(Monitor *monitor) : MatrixPlot(monitor->matrix())
{
	_monitor = monitor;
	_monitor->setResponder(this);
}

void PathPlot::makeImage(int idx, glm::vec3 pos, std::string str)
{
	clearImage(idx);
	Image *image = new Image(str);
	image->resize(0.05);
	image->setPosition(pos);

	_mutex.lock();
	addObject(image);
	_images[idx] = image;
	_mutex.unlock();
}

void PathPlot::clearImage(int idx)
{
	_mutex.lock();
	
	if (_images.count(idx) == 0)
	{
		_mutex.unlock();
		return;
	}

	Image *toGo = _images[idx];
	removeObject(toGo);
	Window::setDelete(toGo);
	_images.erase(idx);

	_mutex.unlock();
}

void PathPlot::sendObject(std::string tag, void *object)
{
	Monitor::StatusInfo info = *static_cast<Monitor::StatusInfo *>(object);

	int full = mat().cols * info.x + info.y;

	if (info.status == TaskType::None)
	{
		clearImage(full);
		return;
	}
	
	
	if (_index2Vertex.count(full) == 0)
	{
		return;
	}

	int start_idx = _index2Vertex[full];
	
	glm::vec3 ave{};
	for (size_t i = 0; i < _vertices.size() && i < 4; i++)
	{
		size_t idx = i + start_idx;
		ave += _vertices[idx].pos;
	}
	
	ave /= 4;

	if (info.status == Validation)
	{
		makeImage(full, ave, "assets/images/eye.png");
	}
	else if (info.status == Optimisation)
	{
		makeImage(full, ave, "assets/images/tools.png");
	}
}
