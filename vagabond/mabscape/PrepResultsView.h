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

#ifndef __vagabond__PrepResultsView__
#define __vagabond__PrepResultsView__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/gui/elements/Scene.h>
#include <functional>
#include <string>
#include <vector>

// something done while preparing the workspace, flagged if it needs a look
struct PrepMessage
{
	std::string text;
	bool warning{};
	// clicking the warning symbol calls this with the scene it is in
	std::function<void(Scene *)> show{};
};

/** the jobs completed while preparing the workspace, a page at a time, with
 *  a warning symbol beside any that need attention. "Next" calls the job
 *  with this scene, for the next scene to come after it. */
class PrepResultsView : public ListView
{
public:
	PrepResultsView(Scene *prev, const std::vector<PrepMessage> &messages,
	                const std::function<void(Scene *)> &next);

	virtual void setup();
	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
private:
	std::vector<PrepMessage> _messages;
	std::function<void(Scene *)> _next;
};

#endif
