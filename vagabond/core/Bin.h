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

#ifndef __vagabond__Bin__
#define __vagabond__Bin__

#include "engine/Handler.h"
#include "engine/Task.h"

template <typename I, typename O> class Task;

template <class Object>
class Bin : public Handler
{
public:
	void submit(Object *r)
	{
		_objectPool.pushObject(r);

		// an extra signal because we trapped this semaphore when submitting
		// the corresponding job. IS THIS A BUG?
		_objectPool.one_arrived();
	}

	void submit(const Object &r)
	{
		Object *remade = new Object(r);
		_objectPool.pushObject(remade);

		// an extra signal because we trapped this semaphore when submitting
		// the corresponding job. IS THIS A BUG?
		_objectPool.one_arrived();
	}

	/** do not let the semaphore unlock itself if it receives all objects */
	void holdHorses()
	{
		_objectPool.expect_one();
	}

	/** if all objects have been queued up now and the semaphore may relax */
	void releaseHorses()
	{
		_objectPool.release_one();
	}

	/** a task representing the submission of an object */
	Task<Object, void *> *actOfSubmission(int ticket)
	{
		auto send = [this, ticket](Object result) -> void *
		{
			result.ticket = ticket;
			submit(result);
			return nullptr;
		};

		_objectPool.expect_one();
		auto *submit = new Task<Object, void *>(send, "submit to bin");
		return submit;

	}
	
	Object *acquireObject()
	{
		Object *object = nullptr;
		_objectPool.acquireObjectOrNull(object);
		return object;
	}
private:
	ExpectantPool<Object *> _objectPool;
private:

};

#endif
