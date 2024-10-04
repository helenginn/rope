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

#ifndef __vagabond__Blame__
#define __vagabond__Blame__

#include <vagabond/utils/OpSet.h>
#include "PathGroup.h"
#include "Progressor.h"
#include "ResidueId.h"
#include "Instance.h"
#include <map>
#include <mutex>

struct Contacts;
class Instance;

class Blame : public Progressor, public HasResponder<Responder<Blame>>
{
public:
	Blame(const std::vector<PathGroup> &paths);

	void process();
	
	virtual const std::string progressName() const
	{
		return "Calculating contacts";
	}
	
	std::pair<int, int> index(Instance *start, Instance *end);
	std::pair<std::string, std::string> names(int l, int r);
	
	float sum_for_residue(Path *const &path, ResidueId residue);

	size_t pathCount()
	{
		return _paths.size();
	}
	
	int rows()
	{
		return _starts.size();
	}

	int cols()
	{
		return _ends.size();
	}
	
	std::vector<PathGroup> &paths()
	{
		return _paths;
	}
	
	std::mutex &mutex()
	{
		return _mutex;
	}

	struct compare_ids
	{
		bool operator()(Instance *const &a, Instance *const &b) const
		{
			return a->id() < b->id();
		}
	};
	
	typedef std::set<Instance *, compare_ids> InstanceSet;
private:
	std::vector<PathGroup> _paths;
	

	InstanceSet _starts;
	InstanceSet _ends;
	
	std::map<Path *, Contacts> _contacts;
	std::mutex _mutex;
};

#endif
