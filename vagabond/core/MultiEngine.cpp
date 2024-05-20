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

#include "MultiSimplex.h"
#include "engine/Tasks.h"

MultiEngine::MultiEngine()
{
	_immediate = new Tasks();
	_immediate->prepare_threads(1);
	_immediate->name = "immediate";
	_hanging = new Tasks();
	_hanging->prepare_threads(1);
	_hanging->back_default = true;
	_hanging->name = "hanging";
}

void MultiEngine::clearTasks()
{
	_immediate->clear();
	_hanging->clear();

}

MultiEngine::~MultiEngine()
{
	_immediate->cleanup();
	_hanging->cleanup();
	delete _immediate;
	delete _hanging;
	_immediate = nullptr;
	_hanging = nullptr;
}

void MultiEngine::addImmediateTask(BaseTask *bt)
{
	_immediate->addTask(bt);
}

void MultiEngine::addHangingTask(BaseTask *bt)
{
	_hanging->addTask(bt);
}
