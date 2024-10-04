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

#include "Blame.h"
#include "Path.h"
#include "PlausibleRoute.h"

Blame::Blame(const std::vector<PathGroup> &paths)
{
	_paths = paths;
	_ticks = 0;

	for (const PathGroup &group : paths)
	{
		for (Path *const &path : group)
		{
			_starts.insert(path->startInstance());
			_ends.insert(path->endInstance());
			_ticks++;
		}
	}
}

void Blame::process()
{
	for (const PathGroup &group : _paths)
	{
		for (Path *const &path : group)
		{
			PlausibleRoute *pr = path->toRoute();
			pr->setup();

			Contacts contacts = pr->contactMap();
			{
				std::unique_lock<std::mutex> lock(mutex());
				_contacts[path] = contacts;
			}

			path->cleanupRoute();
			clickTicker();
			HasResponder<Responder<Blame>>::sendResponse("update_path", path);
		}
	}
	
	finishTicker();
}

std::pair<int, int> Blame::index(Instance *start, Instance *end)
{
	int st_idx = -1;
	int end_idx = -1;

	auto fix_value = [](const Blame::InstanceSet &insts, Instance *inst,
	                    int &idx)
	{
		idx = -1;
		int n = 0;
		for (Instance *const &check : insts)
		{
			if (inst == check)
			{
				idx = n;
				break;
			}
			n++;
		}
	};
	
	fix_value(_starts, start, st_idx);
	fix_value(_ends, end, end_idx);
	return {st_idx, end_idx};
}	

float Blame::sum_for_residue(Path *const &path, ResidueId residue)
{
	std::unique_lock<std::mutex> lock(mutex());

	if (_contacts.count(path) == 0)
	{
		return NAN;
	}

	Contacts &contacts = _contacts[path];

	std::map<ResidueId, float> for_res = contacts[residue];
	
	float sum = 0;
	for (auto it = for_res.begin(); it != for_res.end(); it++)
	{
		sum += it->second;
	}

	return exp(-sum / 4);
}

std::pair<std::string, std::string> Blame::names(int l, int r)
{
	std::string st_name;
	std::string end_name;

	auto fix_string = [](const Blame::InstanceSet &insts, int idx)
	{
		int n = 0;
		for (Instance *const &check : insts)
		{
			if (n == idx)
			{
				return check->id();
			}
			n++;
		}
		return std::string("");
	};
	
	st_name = fix_string(_starts, l);
	end_name = fix_string(_ends, r);
	
	return {st_name, end_name};
}
