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

	size_t objectCount()
	{
		return _objects.size();
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

	virtual T *insertIfUnique(T &m) { return nullptr; };

	friend void to_json(json &j, const Manager<T> &value);
	friend void from_json(const json &j, Manager<T> &value);
protected:

	typename std::list<T> _objects;
};

#endif
