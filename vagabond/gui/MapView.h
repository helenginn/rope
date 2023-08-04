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
class TorsionWarp;
class WarpControl;
class MatrixPlot;
class TextButton;
class ShowMap;
class Network;
class Warp;

class MapView : public Display, public Responder<Warp>
{
public:
	MapView(Scene *prev, Entity *entity, std::vector<Instance *> instances,
	        Instance *reference);
	virtual ~MapView();

	void showWarpSpace();
	virtual void setup();
	virtual void doThings();
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseMoveEvent(double x, double y);
	virtual void sendObject(std::string, void *object);
	
	void supplyExisting(SpecificNetwork *spec);

	void buttonPressed(std::string tag, Button *button);
private:
	void makeMapping();
	void loadSpace(std::string filename);
	void stopWorker();
	void startWorker();
	void startPlot();

	void skipJob();
	void adjustToLeft(Renderable *r);
	void displayDistances(PCA::Matrix &dist);
	bool sampleFromPlot(double x, double y);
	float plotPosition(float x, float y);
	void plotIndices();
	void toggleAtomDisplayType();
	void makePausable();
	bool _editing = false;

	void addButtons();
	void cleanupPause();
	void saveSpace(std::string filename);
	void askForFilename();

	MatrixPlot *_plot = nullptr;
	std::atomic<bool> _updatePlot{false};
	std::atomic<bool> _refined{true};

	Cartographer *_cartographer = nullptr;
	SpecificNetwork *_specified = nullptr;
	Instance *_reference = nullptr;
	std::vector<Instance *> _instances;
	std::mutex _mutex;

	MatrixPlot *_distances = nullptr;
	PCA::Matrix _distMat{};

	PCA::Matrix _warpMat{};
	float _divisions = 50;
	
	TextButton *_toggle = nullptr;
	TextButton *_refine = nullptr;

	Warp *_warp = nullptr;
	TorsionWarp *_torsionWarp = nullptr;
	WarpControl *_wc = nullptr;

	std::thread *_worker = nullptr;
	bool _updateButtons = false;
	std::atomic<bool> _cleanupPause{false};
	std::vector<float> _last;
};

#endif
