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

#ifndef __vagabond__ToggleFixedView__
#define __vagabond__ToggleFixedView__

#include <vagabond/gui/elements/ListView.h>
#include <functional>

class Positions;

/** a tickbox for each antibody which has structural information: ticked
 *  antibodies are fixed at their structural position for this session,
 *  unticked ones are free to be refined. */
class ToggleFixedView : public ListView
{
public:
	ToggleFixedView(Scene *prev, Positions &positions);

	virtual void setup();
	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);

	// called after every tick
	void setChangedJob(const std::function<void()> &job)
	{
		_changed = job;
	}
private:
	Positions &_positions;
	std::vector<std::string> _names;
	std::function<void()> _changed;
};

#endif
