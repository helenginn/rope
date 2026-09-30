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
#include <memory>

class LineGroup;
class ScrollBox;
class Scene;

/** list of every cached refinement Result, lowest score first, on the
 *  left hand side of the screen. Clicking a Result calls the "apply" job. */
class ResultsView : public Image
{
public:
	typedef std::function<void(const PosToComp::Result &)> ApplyJob;

	typedef std::function<void(const std::vector<PosToComp::Result> &)>
	ClusterJob;

	// called with a group's results when it is selected (left click), or
	// with an empty list when it is deselected again
	typedef ClusterJob SelectJob;

	ResultsView(Scene *scene, const ApplyJob &apply);

	// called (after confirmation) with the number of results the list
	// is showing, when "delete" is chosen from the group menu
	void setDeleteJob(const std::function<void(size_t)> &job)
	{
		_delete = job;
	}

	// called with a group's results, in score order, when "cluster" is chosen
	void setClusterJob(const ClusterJob &job)
	{
		_cluster = job;
	}

	void setSelectJob(const SelectJob &job)
	{
		_selectJob = job;
	}

	// adds a named group of results (sorted by score) alongside "All", and
	// folds "All" away
	void addGroup(const std::string &name,
	              const std::vector<PosToComp::Result> &results);

	void setKillAndClean(const std::function<void()> &do_it)
	{
		_kill = do_it;
	}

	// replaces the list with these Results, sorted by score.
	void update(const std::vector<PosToComp::Result> &results);
private:
	void setupCloseButton();
	struct Group;
	// group == nullptr means "All"
	void groupMenu(Group *group);
	void selectGroup(Item *item, const std::vector<PosToComp::Result> &r);
	void rename(Group *group);
	void refreshName(Group *group);
	void refreshAllName();
	std::vector<PosToComp::Result> sortedResults() const;
	std::vector<size_t> sortedOrder() const;

	Scene *_scene{};
	ApplyJob _apply;
	ClusterJob _cluster;
	std::function<void(size_t)> _delete;
	std::function<void()> _kill{};

	struct Group
	{
		std::string name;
		Item item;
		std::vector<PosToComp::Result> results;
		std::vector<std::unique_ptr<Item>> items;
	};

	SelectJob _selectJob;
	Item *_selected{};
	std::string _allName = "All";
	Item _root{};
	Item _parent{};
	std::vector<std::unique_ptr<Group>> _groups;
	LineGroup *_lg{};
	ScrollBox *_sb{};

	// _items[i] is the row for _results[i]; rows beyond _results.size() are
	// detached and kept for reuse
	std::vector<PosToComp::Result> _results;
	std::vector<std::unique_ptr<Item>> _items;
};

#endif
