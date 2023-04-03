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

#ifndef __vagabond__ThreadWorksOnModel__
#define __vagabond__ThreadWorksOnModel__

#include "ThreadWorksOnModel.h"
#include "SerialJob.h"
#include "Model.h"

ThreadWorksOnModel::ThreadWorksOnModel(SerialJob<Model *, ThreadWorksOnModel> 
                                       *handler)
: ThreadWorksOnObject<ThreadWorksOnModel, Model *>(handler)
{

}

bool ThreadWorksOnModel::doJob(Model *model)
{
	model->load();
	
	if (watching())
	{
		_handler->attachObject(model);
	}
	
	_handler->updateObject(model, _num);

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

	_handler->updateObject(nullptr, _num);
	_handler->incrementFinished();

	if (watching())
	{
		_handler->detachObject(model);
	}
	
	model->unload();
	return true;
}

#endif
