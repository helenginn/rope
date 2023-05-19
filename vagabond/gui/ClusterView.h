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
#include <mutex>
#include "PointyView.h"
#include <vagabond/core/Progressor.h>
#include <vagabond/core/Manager.h>

class Path;
class Rule;
class PathView;
class HasMetadata;
class FloatingText;
class MetadataGroup;
class RopeCluster;
class ConfSpaceView;

class ClusterView : public PointyView,
public Progressor,
public Responder<Manager<Path>>
{
public:
	ClusterView();
	~ClusterView();

	void setCluster(RopeCluster *cx);
	
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
	
	RopeCluster *cluster()
	{
		return _cx;
	}

	virtual void makePoints();
	virtual void updatePoints();
	virtual void additionalJobs();
	
	void setInherit(ClusterView *cv)
	{
		_inherit = cv;
	}
	
	void addPath(Path *path, std::vector<PathView *> *refreshers = nullptr);

	virtual void click(bool left = true);

	void applyRule(const Rule &r);
	void applySelected();
	static void populatePaths(ClusterView *me, std::vector<PathView *> pvs);

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
protected:

private:
	void clearPaths();
	void addPaths();
	void removeMissingPaths(std::vector<PathView *> *refreshers);
	void privatePopulatePaths(std::vector<PathView *> pvs);

	void addPathView(PathView *pv);
	void clearOldPathView(PathView *pv);

	static void invertSVD(ClusterView *me);
	void wait();
	void passiveWait();
	void waitForInvert();

	void applyVaryColour(const Rule &r);
	void applyChangeIcon(const Rule &r);
	
	bool _updatingProgressBar = false;
	
	void setFinish(bool finish);

	RopeCluster *_cx = nullptr;
	FloatingText *_text = nullptr;
	ConfSpaceView *_confSpaceView = nullptr;

	std::map<const Rule *, std::vector<HasMetadata *>> _members;
	std::map<int, int> _point2Index;
	
	std::map<Path *, PathView *> _path2View;
	std::vector<PathView *> _pathViews;
	std::thread *_worker = nullptr;
	std::thread *_invert = nullptr;

	std::atomic<bool> _finish{false}, _running{false};
	std::condition_variable _cv;
	
	ClusterView *_inherit = nullptr;
	std::mutex _lockPopulating;
	
	int _clusterVersion = 0;
};

#endif
