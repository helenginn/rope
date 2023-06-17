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

#ifndef __vagabond__MapView__
#define __vagabond__MapView__

#include <vagabond/gui/Display.h>
#include <vagabond/utils/Mapping.h>
#include <vagabond/core/Responder.h>
#include <thread>

class MappingToMatrix;
class SpecificNetwork;
class Cartographer;
class MatrixPlot;
class TextButton;
class Network;

class MapView : public Display, public Responder<Cartographer>,
public Responder<SpecificNetwork>
{
public:
	MapView(Scene *prev, Entity *entity, std::vector<Instance *> instances);
	virtual ~MapView();

	void makeTriangles();
	virtual void setup();
	virtual void doThings();
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseMoveEvent(double x, double y);
	virtual void sendObject(std::string, void *object);

	void buttonPressed(std::string tag, Button *button);
private:
	void makeMapping();
	void startFlips();
	void startNudges();
	void stopWorker();
	void skipJob();
	void displayDistances(PCA::Matrix &dist);
	bool sampleFromPlot(double x, double y);
	void makePausable();
	bool _editing = false;

	void addButtons();
	void cleanupPause();

	MatrixPlot *_plot = nullptr;
	std::atomic<bool> _updatePlot{false};
	std::atomic<bool> _refined{false};

	Cartographer *_cartographer = nullptr;
	SpecificNetwork *_specified = nullptr;
	std::mutex _mutex;

	MatrixPlot *_distances = nullptr;
	PCA::Matrix _distMat{};
	
	TextButton *_command = nullptr;
	TextButton *_second = nullptr;

	std::thread *_worker = nullptr;
	bool _updateButtons = false;
	std::atomic<bool> _cleanupPause{false};
};

#endif
