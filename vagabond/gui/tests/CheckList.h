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

#ifndef __vagabond__CheckList__
#define __vagabond__CheckList__

#include <vector>
#include <string>

#define WAITCOUNT (30)

class CheckList
{
public:
	CheckList(const std::string &filename);

	bool next();
private:
	void processCommand(const std::string &line);
	void pushClick(const std::vector<std::string> &bits);
	void pushKey(const std::vector<std::string> &bits);

	struct Progress
	{
		Progress()
		{
			
		}
		
		int total()
		{
			return commands.size();
		}

		std::vector<std::string> commands;
		int number = -1;
		int countdown = WAITCOUNT;
	};

	Progress _progress;
	bool _done = false;
	int _waitCount = WAITCOUNT;
};

#endif
