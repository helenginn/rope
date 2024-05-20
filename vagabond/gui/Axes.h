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
#include <vagabond/c4x/ClusterSVD.h>
#include <vagabond/core/Engine.h>
#include <vagabond/c4x/Posular.h>
#include <thread>
#include "RTAngles.h"

class Scene;
class HasMetadata;
class ClusterSVD;
class ChemotaxisEngine;
class ConfSpaceView;
class RopeCluster;

class Axes : public IndexResponder, public ButtonResponder, public RunsEngine
{
public:
	Axes(ObjectData *group, ClusterSVD *cluster, HasMetadata *m = nullptr);
	~Axes();
	
	void setScene(ConfSpaceView *scene)
	{
		_scene = scene;
	}
	
	enum Status
	{
		Unedited,
		PrincipalCombination,
		PointingToOther,
		None
	};

	virtual void interacted(int idx, bool hover, bool left = true);
	virtual void reindex();
	void reorient(int i, HasMetadata *mol);
	virtual void click(bool left = true);
	virtual bool mouseOver();
	virtual void unmouseOver();

	virtual size_t requestedIndices();
	
	void takeOldAxes(Axes *old);

	void buttonPressed(std::string tag, Button *button);
	void backgroundPrioritise(std::string key);
	
	Status status(int idx = -1)
	{
		if (idx == -1)
		{
			idx = _lastIdx;
		}
		if (idx == -1)
		{
			return None;
		}

		if (_targets[idx])
		{
			return PointingToOther;
		}
		
		return (_edited[idx] ? PrincipalCombination : Unedited);
	}
	
	glm::vec3 axisInQuestion()
	{
		if (_lastIdx < 0) return {};
		return _dirs[_lastIdx];
	}
	
	ClusterSVD *const &cluster() const
	{
		return _cluster;
	}
	
	ObjectData *const &data() const
	{
		return _data;
	}
	
	HasMetadata *focus()
	{
		return _focus;
	}
	
	ConfSpaceView *scene()
	{
		return _scene;
	}

	std::string titleForAxis(int idx = -1);
	
	void indicesOfObjectsPointedAt(int &start, int &end, int idx = -1);

	virtual size_t parameterCount(Engine *caller);
	virtual int sendJob(const std::vector<float> &all, Engine *caller = nullptr);
	
	virtual bool selectable() const
	{
		return false;
	}
private:
	std::vector<float> getMappedVector(int idx);
	void prioritiseDirection(std::string key);
	void route(int idx);
	void initialise();

	void prepareAxes();
	void refreshAxes();
	void reflect(int i);
	void assembleMenu();

	ObjectData *_data = nullptr;
	ClusterSVD *_cluster = nullptr;
	HasMetadata *_focus = nullptr;
	ConfSpaceView *_scene = nullptr;

	// last accessed arrow, 0 to 2 or -1 if not accessed
	int _lastIdx = -1;
	
	// directions in which the axes are pointing
	HasMetadata *_targets[3];
	
	void stop();
	std::thread *_worker = nullptr;
	std::string _key;
	ChemotaxisEngine *_engine = nullptr;

	bool _edited[3] = {false, false, false};
	std::vector<glm::vec3> _dirs;
	
};

#endif
