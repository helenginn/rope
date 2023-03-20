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

#include "engine/workers/ThreadWorksOnModel.h"
#include "SerialJob.h"
#include "Model.h"

ThreadWorksOnModel::ThreadWorksOnModel(SerialJob *handler)
{
	_handler = handler;
}

void ThreadWorksOnModel::doJob(Model *model)
{
	model->load();
	
	if (watching())
	{
		_handler->attachModel(model);
	}
	
	_handler->updateModel(model, _num);

	if (_job == rope::Refine)
	{
		model->refine(true);
	}
	if (_job == rope::ThoroughRefine)
	{
		model->refine(true, true);
	}
	else if (_job == rope::SkipRefine)
	{
		model->extractExisting();
	}

	_handler->updateModel(nullptr, _num);
	_handler->incrementFinished();

	if (watching())
	{
		_handler->detachModel(model);
	}
	
	model->unload();
}

void ThreadWorksOnModel::start()
{
	do
	{
		Model *model = _handler->acquireModel();
		
		if (model == nullptr)
		{
			break;
		}
		
		timeStart();
		doJob(model);
		timeEnd();

	}
	while (!_finish);
}
