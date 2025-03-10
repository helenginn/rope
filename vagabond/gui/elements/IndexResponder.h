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

#ifndef __vagabond__IndexResponder__
#define __vagabond__IndexResponder__

#include <vagabond/gui/elements/SimplePolygon.h>
#include "IndexResponseView.h"

// you need to reimplement requestedIndices() and reindex()
// and prepare to get signals on interacted(...)
// addIndexResponder for each of the responders
// also IndexResponseView::setup needs calling

class IndexResponder : public SimplePolygon
{
public:
	virtual ~IndexResponder() {};
	
	void setIndexResponseView(IndexResponseView *irv)
	{
		_view = irv;
	}
	
	void clearView()
	{
		_view = nullptr;
	}

	virtual void interacted(int idx, bool hover, bool left) {};
	virtual void selected(int idx, bool inverse) {};
	virtual void reindex() {};
	
	virtual bool selectable() const
	{
		return false;
	}

	virtual size_t requestedIndices() = 0;
	
	virtual bool index_in_range(int idx, float t, float l, float b, float r)
	{
		return false;
	}

	virtual size_t indexOffset()
	{
		if (_view == nullptr)
		{
			return 0;
		}

		return _view->indexOffset(this);
	}
protected:
	IndexResponseView *_view = nullptr;

};

#endif
