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

#ifndef __vagabond__ResultsView__
#define __vagabond__ResultsView__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/Item.h>
#include "PosToComp.h"
#include <functional>
#include <map>
#include <memory>

class LineGroup;
class ScrollBox;
class Scene;

/** list of every cached refinement Result on the left hand side of the
 *  screen, in a bucket for each set of fixed antibodies the results were
 *  made with:
 *
 *  - Results for fixed abs X, Y
 *  -- All (N results)
 *  -- best (n results)
 *
 *  Within a bucket "All" is lowest score first. Clicking a Result calls the
 *  "apply" job. */
class ResultsView : public Image
{
public:
	typedef std::function<void(const PosToComp::Result &)> ApplyJob;

	// the bucket's key (see bucketKey()) and the results of the group
	typedef std::function<void(const std::string &,
	                           const std::vector<PosToComp::Result> &)>
	ClusterJob;

	// called with the bucket's key and a group's results when it is selected
	// (left click), or with an empty list when it is deselected again
	typedef ClusterJob SelectJob;

	ResultsView(Scene *scene, const ApplyJob &apply);

	// the antibodies which were fixed when this Result was made, e.g.
	// "X, Y" - empty if none were
	static std::string bucketKey(const PosToComp::Result &result);
	// the antibody names in a key
	static std::vector<std::string> keyNames(const std::string &key);

	// called (after confirmation) with a bucket's key and the number of
	// results in it, when "delete" is chosen from the "All" menu
	void setDeleteJob(const std::function<void(const std::string &, 
	                                           size_t)> &job)
	{
		_delete = job;
	}

	// called with a group's bucket and results, in score order, when
	// "cluster" is chosen
	void setClusterJob(const ClusterJob &job)
	{
		_cluster = job;
	}

	// the bucket's key, the group's name and its results, when "cross-check"
	// is chosen
	typedef std::function<void(const std::string &, const std::string &,
	                           const std::vector<PosToComp::Result> &)>
	CrossCheckJob;

	void setCrossCheckJob(const CrossCheckJob &job)
	{
		_crossCheck = job;
	}

	void setSelectJob(const SelectJob &job)
	{
		_selectJob = job;
	}

	// adds a named group of results (sorted by score) alongside "All" in
	// the bucket, and folds "All" away
	void addGroup(const std::string &key, const std::string &name,
	              const std::vector<PosToComp::Result> &results);

	// every group made so far, for saving
	std::vector<ResultGroup> groups() const;

	void setKillAndClean(const std::function<void()> &do_it)
	{
		_kill = do_it;
	}

	// replaces the lists with these Results, sorted into buckets
	void update(const std::vector<PosToComp::Result> &results);
private:
	struct Group
	{
		std::string name;
		Item item;
		std::vector<PosToComp::Result> results; // lowest score first
		std::vector<std::unique_ptr<Item>> items;
	};

	struct Bucket
	{
		std::string key;
		Item item;
		Item allItem;
		std::string allName = "All";
		std::vector<PosToComp::Result> results; // lowest score first
		std::vector<std::unique_ptr<Item>> items;
		std::vector<std::unique_ptr<Group>> groups;
	};

	void setupCloseButton();
	Bucket *bucket(const std::string &key);
	// group == nullptr means the bucket's "All"
	void groupMenu(Bucket *bucket, Group *group);
	void removeGroup(Bucket *bucket, Group *group);
	void selectGroup(Item *item, const std::string &key,
	                 const std::vector<PosToComp::Result> &r);
	void rename(Bucket *bucket, Group *group);
	void refreshNames(Bucket *bucket);

	Scene *_scene{};
	ApplyJob _apply;
	ClusterJob _cluster;
	std::function<void(const std::string &, size_t)> _delete;
	std::function<void()> _kill{};

	CrossCheckJob _crossCheck;
	SelectJob _selectJob;
	Item *_selected{};
	Item _root{};
	// in key order; kept even when emptied, only detached from the list
	std::map<std::string, std::unique_ptr<Bucket>> _buckets;
	// deleted groups are only detached, not destroyed, as with the buckets
	std::vector<std::unique_ptr<Group>> _removed;
	LineGroup *_lg{};
	ScrollBox *_sb{};
};

#endif
