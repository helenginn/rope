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

#ifndef __vagabond__BFactors__
#define __vagabond__BFactors__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>
#include <vagabond/core/MetadataGroup.h>
#include <mutex>

class Entity;
class MatrixPlot;
class ChooseHeader;

class BFactors : public Scene, public Responder<ChooseHeader>
{
public:
	BFactors(Scene *prev, Entity *entity);

	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button);
	virtual void refresh();

	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	void sampleFromPlot(double x, double y);
	void drawPlot();

	Entity *_entity = nullptr;
	Text *_headText = nullptr;
	MatrixPlot *_plot = nullptr;

	std::string _header;
	std::vector<std::pair<Instance *, std::string> > _mapping;
	
	std::mutex _mutex;
	PCA::Matrix _results{};
	MetadataGroup _group = MetadataGroup(0);
};

#endif
