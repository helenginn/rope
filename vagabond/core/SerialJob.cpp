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

#ifndef __vagabond__SerialJob__cpp__
#define __vagabond__SerialJob__cpp__

template <class Obj, class Thr>
SerialJob<Obj, Thr>::SerialJob(SerialJobResponder<Obj> *responder)
{
	_responder = responder;
}

template <class Obj, class Thr>
SerialJob<Obj, Thr>::~SerialJob()
{
	finish();
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::setup()
{
	settings();
	loadObjectsIntoPool();
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::pushObject(Obj &obj)
{
	pool().pushObject(obj);
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::loadObjectsIntoPool()
{
	for (Obj m : _objects)
	{
		pool().pushObject(m);
	}
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::start()
{
	prepareThreads();
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::prepareThreads()
{
	for (size_t i = 0; i < _threads; i++)
	{
		Thr *worker = new Thr(this);
		std::thread *thr = new std::thread(&Thr::start, worker);
		worker->setRopeJob(_job);
		worker->setIndex(i);

		pool().addWorker(worker, thr);
	}
}

template <class Obj, class Thr>
Obj SerialJob<Obj, Thr>::acquireObject()
{
	Obj m = nullptr;
	pool().acquireObject(m);
	return m;
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::finishedObject(Obj obj)
{
	pool().notifyFinishedObject(obj);
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::detachObject(Obj obj)
{
	if (!_responder)
	{
		return;
	}

	_responder->detachObject(obj);
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::attachObject(Obj obj)
{
	if (!_responder)
	{
		return;
	}

	_responder->attachObject(obj);
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::finish()
{
	pool().finish();
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::incrementFinished()
{
	std::unique_lock<std::mutex> lock(_mutex);
	
	_finished++;
	if (objectCount() == _finished)
	{
		if (_responder)
		{
			_responder->finishedObjects();
		}
	}
}

template <class Obj, class Thr>
void SerialJob<Obj, Thr>::updateObject(Obj obj, int idx)
{
	if (_responder)
	{
		_responder->updateObject(obj, idx);
	}
}



#endif
