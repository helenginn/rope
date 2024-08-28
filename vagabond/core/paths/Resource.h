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

#ifndef __vagabond__Resource__
#define __vagabond__Resource__

#include <mutex>
#include <functional>
#include <thread>

// please don't try and request/access resources from different threads, that's
// not supported by this class
template <class Pointer>
class Resource
{
public:
	Resource(const std::function<Pointer *()> &creation = {})
	{
		_creation = creation;
		_mutex = std::shared_ptr<std::mutex>(new std::mutex());
	}
	
	void clear()
	{
		delete _resource;
		_resource = nullptr;
	}
	
	void reset()
	{
		clear();
		acquire();
	}
	
	Pointer *acquire()
	{
		std::unique_lock<std::mutex> lock(*_mutex);
		
		if (_resource)
		{
			return _resource;
		}

		if (!_worker && !_resource)
		{
			request();
		}

		if (_worker)
		{
			_worker->join();
			delete _worker;
			_worker = nullptr;
		}
		
		return _resource;
	}

	void request()
	{
		if (_worker || _resource)
		{
			return;
		}

		auto creation_into_object = [this]()
		{
			if (!_creation)
			{
				std::cout << "Warning! creation function is empty!" << std::endl;
				std::cout << "Crashing soon" << std::endl;
			}
			Pointer *ptr = _creation();
			_resource = ptr;
		};

		_worker = new std::thread(creation_into_object);

	}
private:
	std::function<Pointer *()> _creation{};
	
	Pointer *_resource = nullptr;
	std::thread *_worker = nullptr;
	std::shared_ptr<std::mutex> _mutex = nullptr;
};

#endif
