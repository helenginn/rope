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

#ifndef __vagabond__PathFinderView__
#define __vagabond__PathFinderView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>

class PathFinder;
class LineGroup;
class Summary;
class Box;

class PathFinderView : public Scene, Responder<PathFinder *>
{
public:
	PathFinderView(Scene *scene);
	
	PathFinder *pathFinder() const
	{
		return _pf;
	}

	virtual void setup();
	virtual void doThings();
protected:
	virtual void respond();
private:
	void overviewButtons();
	void makeTaskTree();
	void updateSummary();
	void makeSummary();
	PathFinder *_pf = nullptr;
	LineGroup *_taskTree = nullptr;
	Box *_summaryBox = nullptr;
	Summary *_summary = nullptr;
	
	bool _updateNext = false;
};

#endif
