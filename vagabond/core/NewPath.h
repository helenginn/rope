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

#ifndef __vagabond__NewPath__
#define __vagabond__NewPath__

#include <vector>

class Instance;
class PlausibleRoute;
class TorsionData;
class Path;

class NewPath
{
public:
	NewPath(Instance *from, Instance *to, Path *blueprint = nullptr);
	NewPath(Path *blueprint = nullptr);
	
	void addLinkedInstances(Instance *from, Instance *to);

	PlausibleRoute *operator()();
	TorsionData *addPair(Instance *from, Instance *to);
	TorsionData *dataAllPairs();
private:
	Path *_blueprint = nullptr;

	struct InstancePair
	{
		Instance *start = nullptr;
		Instance *end = nullptr;
	};
	
	std::vector<InstancePair> _pairs;
	Instance *_from = nullptr;
	Instance *_to = nullptr;

};

#endif
