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

#ifndef __vagabond__IndexResponseView__
#define __vagabond__IndexResponseView__

#include "Scene.h"

class IndexResponder;

class IndexResponseView : virtual public Scene
{
public:
	IndexResponseView(Scene *prev);

	virtual void setup();

	virtual const bool hasIndexedObjects() const
	{
		return true;
	}
	
	void addIndexResponder(IndexResponder *ir);
	void removeResponder(IndexResponder *ir);

	size_t indexOffset(IndexResponder *ir);
protected:
	virtual void checkIndexBuffer(double x, double y, bool hover, 
	                              bool arrow, bool left);
	
	virtual void interactedWithNothing(bool left) {};

private:
	IndexResponder *getResponderForIndex(int &val);

	std::vector<IndexResponder *> _responders;
	IndexResponder *_lastResponder = nullptr;
};

#endif
