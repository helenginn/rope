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

#ifndef __vagabond__Manager__
#define __vagabond__Manager__

#include <list>
#include "Progressor.h"
#include "Responder.h"


template <class T>
class Manager : public HasResponder<Responder<Manager<T>>>
{
public:
	Manager() {};
	virtual ~Manager() {};

	size_t objectCount() const
	{
		return _objects.size();
	}
	
	std::list<T *> ptrs() 
	{
		std::list<T *> l;
		
		for (T obj : _objects)
		{
			l.push_back(&obj);
		}

		return l;
	}
	
	std::list<T> &objects()
	{
		return _objects;
	}

	T &object(int idx)
	{
		typename std::list<T>::iterator it = _objects.begin();
		for (size_t i = 0; i < idx; i++, it++) {};
		return *it;
	}
	
	bool remove(T &obj)
	{
		typename std::list<T>::iterator it = _objects.begin();
		
		for (; it != _objects.end(); it++)
		{
			if (&*it == &obj)
			{
				_objects.erase(it);
				HasResponder<Responder<Manager<T>>>::triggerResponse();
				return true;
			}
		}
		
		return false;
	}

	virtual T *insertIfUnique(T &m) { return nullptr; };

protected:

	typename std::list<T> _objects;
};

#endif
