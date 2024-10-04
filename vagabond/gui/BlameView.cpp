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

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/core/paths/Blame.h>
#include <mutex>

#include "MatrixPlot.h"
#include "BlameView.h"
#include "Sequence.h"
#include "Entity.h"
#include "Path.h"

BlameView::BlameView(Scene *prev, Entity *entity,
                     const std::vector<PathGroup> &paths)
: Scene(prev)
{
	_blame = new Blame(paths);
	_blame->HasResponder<Responder<Blame>>::setResponder(this);
	_entity = entity;

	std::cout << _blame->rows() << " x " << _blame->cols() << std::endl;
	PCA::setupMatrix(&_data, _blame->cols(), _blame->rows());
}

void BlameView::setup()
{
	_plot = new MatrixPlot(_data, _mutex);
	addObject(_plot);

	addTitle("Blame (" + std::to_string(_blame->pathCount()) + " paths)");
	addSlider();

	VagWindow::window()->requestProgressBar(_blame->ticks(), 
	                                        "Calculating contacts");

	_worker = new std::thread([this]() { _blame->process(); });
}

void BlameView::mousePressEvent(double x, double y, 
                                SDL_MouseButtonEvent button)
{
	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;

	if ((v.x < 0 || v.x > 1) || (v.y < 0 || v.y > 1))
	{
		Scene::mousePressEvent(x, y, button);
		return;
	}

	int left = v.x * _data.cols;
	int right = v.y * _data.rows;
	std::cout << left << " " << right << std::endl;
	auto names = _blame->names(left, right);
	setInformation(names.first + " vs. " + names.second);
	Scene::mousePressEvent(x, y, button);
}


void BlameView::addSlider()
{
	int min = _entity->sequence()->firstNum();
	_res = min;
	int max = _entity->sequence()->lastNum();
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.7);
	s->setup("Residue number", min, max, 1);
	s->setStart(0.5, 0.);
	s->setCentre(0.5, 0.85);
	addObject(s);
}

void BlameView::sendObject(std::string tag, void *object)
{
	if (tag == "update_path")
	{
		Path *const path = static_cast<Path *const>(object);
		refreshPlot(path);
		_plot->update();
	}
}

void BlameView::finishedDragging(std::string tag, double x, double y)
{
	_res = x;
	refreshPlot();
}

float BlameView::refreshPlot(Path *const &path)
{
	std::pair<int, int> idxs = _blame->index(path->startInstance(),
	                                         path->endInstance());

	float value = _blame->sum_for_residue(path, ResidueId(_res));

	std::unique_lock<std::mutex> lock(_mutex);
	_data[idxs.second][idxs.first] = value;
	return value;
}

void BlameView::refreshPlot()
{
	float sum = 0;
	float tot = 0;
	for (PathGroup &group : _blame->paths())
	{
		for (Path *const &path : group)
		{
			float val = refreshPlot(path);
			if (val == val)
			{
				sum += log(val);
				tot++;
			}
		}
	}
	sum /= tot;
	sum = exp(sum);
	
	if (tot > 0)
	{
		std::unique_lock<std::mutex> lock(_mutex);
//		do_op(_data, [sum](const float &f) { return f / sum; });
	}
	
	_plot->update();
}

