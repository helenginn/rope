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

#ifndef __vagabond__BlameView__
#define __vagabond__BlameView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/svd/PCA.h>
#include <thread>
#include <mutex>

class MatrixPlot;
class PathGroup;
class Blame;

class BlameView : public Scene, public Responder<Blame>, public DragResponder
{
public:
	BlameView(Scene *prev, Entity *entity, const std::vector<PathGroup> &paths);

	virtual void setup();
	virtual void finishedDragging(std::string tag, double x, double y);
	virtual void mousePressEvent(double x, double y, 
	                             SDL_MouseButtonEvent button);
private:
	void refreshPlot();
	float refreshPlot(Path *const &path);
	virtual void sendObject(std::string tag, void *object);

	Blame *_blame{};
	std::thread *_worker = nullptr;
	Entity *_entity = nullptr;
	int _res = 0;

	void addSlider();
	MatrixPlot *_plot{};
	PCA::Matrix _data;
	std::mutex _mutex;
};

#endif
