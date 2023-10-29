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

#ifndef __vagabond__BaseTask__
#define __vagabond__BaseTask__

#include <atomic>
#include <vector>
#include <string>

class BaseTask
{
public:
	virtual std::vector<BaseTask *> operator()() { return {}; };
	virtual ~BaseTask()
	{

	}
	
	bool ready()
	{
		return (signals >= expected);
	}

	std::atomic<int> signals{0};
	int expected = 0;
	int total    = 0;
	int priority = 1;
	std::string name;
	
	std::vector<std::function<BaseTask *()>> connections;
};

#endif
