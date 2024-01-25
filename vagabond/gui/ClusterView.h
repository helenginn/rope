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

#ifndef __vagabond__ClusterView__
#define __vagabond__ClusterView__

#include <thread>
#include <condition_variable>
#include <mutex>
#include <condition_variable>
#include "PointyView.h"
#include <vagabond/core/Progressor.h>
#include <vagabond/core/Manager.h>

class Path;
class Rule;
class HasMetadata;
class FloatingText;
class MetadataGroup;
class ClusterSVD;
class ObjectGroup;
class ConfSpaceView;

class ClusterView : public PointyView,
public Progressor,
public Responder<Manager<Path>>
{
public:
	ClusterView();
	~ClusterView();

	void setCluster(ClusterSVD *cx, ObjectGroup *data);
	
	void clearRules()
	{
		_members.clear();
	}

	void setConfSpaceView(ConfSpaceView *csv)
	{
		_confSpaceView = csv;
	}
	
	std::vector<HasMetadata *> &membersForRule(Rule *r)
	{
		return _members[r];
	}
	
	std::vector<HasMetadata *> selectedMembers();
	
	ClusterSVD *cluster()
	{
		return _cx;
	}
	
	ObjectGroup *data()
	{
		return _data;
	}

	virtual void makePoints();
	virtual void updatePoints();
	virtual void additionalJobs();
	
	void setInherit(ClusterView *cv)
	{
		_inherit = cv;
	}

	virtual void click(bool left = true);

	void applyRule(const Rule &r);
	void applySelected();

	void prioritiseMetadata(std::string key);
	
	virtual void interacted(int idx, bool hover, bool left = true);
	virtual void selected(int idx, bool inverse);

	virtual void respond();
	
	virtual bool selectable() const
	{
		return true;
	}
	
	void reselect();
	void deselect();
private:
	static void invertSVD(ClusterView *me);
	void wait();
	void passiveWait();
	void waitForInvert();

	void applyVaryColour(const Rule &r);
	void applyChangeIcon(const Rule &r);
	
	bool _updatingProgressBar = false;
	
	void setFinish(bool finish);

	ClusterSVD *_cx = nullptr;
	FloatingText *_text = nullptr;
	ConfSpaceView *_confSpaceView = nullptr;

	std::map<const Rule *, std::vector<HasMetadata *>> _members;
	std::map<int, int> _point2Index;
	
	std::thread *_worker = nullptr;
	std::thread *_invert = nullptr;

	std::atomic<bool> _finish{false}, _running{false};
	std::condition_variable _cv;
	
	ObjectGroup *_data = nullptr;
	ClusterView *_inherit = nullptr;
	std::mutex _lockPopulating;
	
	int _clusterVersion = 0;
};

#endif
