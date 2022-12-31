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

#include "SerialJob.h"
#include "Entity.h"
#include "engine/workers/ThreadWorksOnModel.h"

SerialJob::SerialJob(Entity *entity, SerialJobResponder *responder)
{
	_entity = entity;
	_responder = responder;
}

void SerialJob::setup()
{
	settings();
}

void SerialJob::start()
{
	prepareThreads();
}

void SerialJob::settings()
{
	_entity->setActuallyRefine(_job != rope::SkipRefine);

	if (_models.size() == 0)
	{
		_models = _entity->models();
	}
	
	for (Model *m : _models)
	{
		_pool.pushObject(m);
	}
}

void SerialJob::prepareThreads()
{
	for (size_t i = 0; i < _threads; i++)
	{
		ThreadWorksOnModel *worker = new ThreadWorksOnModel(this);
		std::thread *thr = new std::thread(&ThreadWorksOnModel::start, worker);
		worker->setRopeJob(_job);
		worker->setWatch(i == 0);

		_pool.threads.push_back(thr);
		_pool.workers.push_back(worker);
	}
}

Model *SerialJob::acquireModel()
{
	Model *m = nullptr;
	_pool.acquireObject(m, _finish);
	return m;

}

void SerialJob::detachModel(Model *model)
{
	if (!_responder)
	{
		return;
	}

	_responder->detachModel(model);
}

void SerialJob::attachModel(Model *model)
{
	if (!_responder)
	{
		return;
	}

	_responder->attachModel(model);
}

void SerialJob::waitToFinish()
{
	_pool.waitForThreads();
	
	if (_responder)
	{
		_responder->finishedModels();
	}
}
