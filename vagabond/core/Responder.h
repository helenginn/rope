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

#ifndef __vagabond__Responder__
#define __vagabond__Responder__

#include <algorithm>
#include <vector>
#include <iostream>

template <class R>
class HasResponder;

template <class T>
class Responder
{
public:
	virtual void respond() {};

	virtual ~Responder()
	{
		for (HasResponder<Responder<T>> *parent : _parents)
		{
			parent->removeResponder(this);
		}
	}
	
	void removeParent(HasResponder<Responder<T>> *parent)
	{
		for (size_t i = 0; i < _parents.size(); i++)
		{
			if (_parents[i] == parent)
			{
				_parents.erase(_parents.begin() + i);
				i--;
			}
		}

	}

	virtual void setParent(HasResponder<Responder<T>> *hr)
	{
		_parents.push_back(hr);
	}

	virtual void sendObject(std::string tag, void *object) {}
private:
	std::vector<HasResponder<Responder<T>> *> _parents;

};

template <class R>
class HasResponder
{
public:
	virtual ~HasResponder()
	{
		for (size_t i = 0; i < _responders.size(); i++)
		{
			_responders[i]->removeParent(this);
		}

	}

	virtual void setResponder(R *r)
	{
		if (r == nullptr)
		{
			return;
		}
		
		if (std::find(_responders.begin(), _responders.end(), r) 
		    != _responders.end())
		{
			return;
		}

		_responders.push_back(r);
		r->setParent(this);
	}
	
	void clearResponders()
	{
		while (_responders.size() > 0)
		{
			removeResponder(_responders[0]);
		}
	}
	
	virtual void removeResponder(R *r)
	{
		for (size_t i = 0; i < _responders.size(); i++)
		{
			R *cast = static_cast<R *>(_responders[i]);
			if (cast == r)
			{
				_responders.erase(_responders.begin() + i);
			}
		}
	}
protected:

	virtual void sendResponse(std::string tag, void *object)
	{
		for (R *responder : _responders)
		{
			responder->sendObject(tag, object);
		}
	}

	virtual void triggerResponse()
	{
		for (R *responder : _responders)
		{
			responder->respond();
		}
	}

private:
	std::vector<R *> _responders;

};


#endif
