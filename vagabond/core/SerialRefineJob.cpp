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

#ifndef __vagabond__SerialRefineJob__cpp__
#define __vagabond__SerialRefineJob__cpp__

#include "SerialRefineJob.h"
#include "Environment.h"
#include "FileManager.h"
#include "Entity.h"

SerialRefineJob::SerialRefineJob(Entity *entity,
                                 SerialJobResponder<Model *> *responder) :
SerialJob<Model *, ThreadWorksOnModel>(responder)
{
	_entity = entity;
}

void SerialRefineJob::settings()
{
	_entity->setActuallyRefine(_job != rope::SkipRefine);

	if (_objects.size() == 0)
	{
		_objects = _entity->models();
	}

	Environment::fileManager()->preFilter();
}

#endif
