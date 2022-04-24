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

#include "ModelManager.h"
#include <iostream>

ModelManager::ModelManager()
{

}

void ModelManager::insertIfUnique(const Model &m)
{
	for (Model &other : _models)
	{
		if (other.name() == m.name())
		{
			throw std::runtime_error("Model with same name exists");
		}
	}
	
	if (m.name().length() == 0)
	{
		throw std::runtime_error("Model has no name");
	}
	
	if (m.filename().length() == 0)
	{
		throw std::runtime_error("Model has no initial file");
	}
	
	_models.push_back(m);

	if (_responder)
	{
		_responder->modelsChanged();
	}
}
