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

#ifndef __vagabond__Axes__
#define __vagabond__Axes__

#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/c4x/Cluster.h>
#include <vagabond/core/Engine.h>
#include <thread>

class Scene;
class Instance;
class TorsionCluster;
class PositionalCluster;
class ChemotaxisEngine;
class ConfSpaceView;
class RopeCluster;

class Axes : public IndexResponder, public ButtonResponder, public RunsEngine
{
public:
	Axes(TorsionCluster *group, Instance *m = nullptr);
	Axes(PositionalCluster *group, Instance *m = nullptr);
	Axes(RopeCluster *group, Instance *m = nullptr);
	~Axes();
	
	void setScene(ConfSpaceView *scene)
	{
		_scene = scene;
	}

	virtual void interacted(int idx, bool hover, bool left = true);
	virtual void reindex();
	virtual void reorient(int i, Instance *mol);
	virtual void click(bool left = true);
	virtual bool mouseOver();
	virtual void unmouseOver();
	std::vector<Angular> directTorsionVector(int idx);

	virtual size_t requestedIndices();
	
	void takeOldAxes(Axes *old);

	void buttonPressed(std::string tag, Button *button);
	void backgroundPrioritise(std::string key);

	virtual size_t parameterCount();
	virtual int sendJob(std::vector<float> &all);
private:
	std::vector<float> getMappedVector(int idx);
	void prioritiseDirection(std::string key);
	void route(int idx);
	void initialise();

	std::vector<Angular> getTorsionVector(int axis);
	void prepareAxes();
	void refreshAxes();
	void reflect(int i);
	void loadAxisExplorer(int idx);

	RopeCluster *_cluster = nullptr;
	TorsionCluster *_torsionCluster = nullptr;
	PositionalCluster *_positionalCluster = nullptr;
	Instance *_instance = nullptr;
	ConfSpaceView *_scene = nullptr;
	int _lastIdx = -1;
	
	Instance *_targets[3];
	
	bool _origin = false;

	void stop();
	std::thread *_worker = nullptr;
	std::string _key;
	ChemotaxisEngine *_engine = nullptr;

	bool _planes[3];
	std::vector<glm::vec3> _dirs;
	
};

#endif
