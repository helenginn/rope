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

#ifndef __vagabond__TickList__
#define __vagabond__TickList__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/Responder.h>
#include <map>

class TickList : public ListView, public HasResponder<Responder<TickList>>
{
public:
	TickList(Scene *prev);

	void setOptions(const std::vector<std::string> &headers)
	{
		_headers = headers;
	}

	void setTicked(const std::vector<std::string> &ticked)
	{
		for (const std::string &t : ticked)
		{
			_ticks[t] = true;
		}
	}

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button);
private:
	std::vector<std::string> _headers;
	
	std::map<std::string, bool> _ticks;
};

#endif
