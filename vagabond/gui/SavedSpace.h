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

#ifndef __vagabond__SavedSpace__
#define __vagabond__SavedSpace__

#include <vagabond/core/ConfType.h>
#include <vagabond/core/Environment.h>
#include <map>

class Entity;
class Metadata;
class RopeSpaceItem;

class SavedSpace
{
public:
	static SavedSpace &defaultSpace()
	{
		return _defaultSpace;
	}
	
	SavedSpace();
	
	Metadata *associatedMetadata()
	{
		if (!_metadata)
		{
			return Environment::metadata();
		}
		
		else return _metadata;
	}

	void addAssociatedMetadata(Metadata *metadata);
	
	void do_on_all_spaces(Entity *entity, 
	                      const std::function<void(RopeSpaceItem *)> &job);

	RopeSpaceItem *const load(Entity *entity, const rope::ConfType &type) const
	{
		if (_savedSpaces.count(entity) == 0 ||
		    _savedSpaces.at(entity).count(type) == 0)
		{
			return nullptr;
		}

		return _savedSpaces.at(entity).at(type);
	}
	
	void save(RopeSpaceItem *item, Entity *entity, const rope::ConfType &type)
	{
		_savedSpaces[entity][type] = item;
	}
private:
	std::map<Entity *, std::map<rope::ConfType, RopeSpaceItem *>> _savedSpaces;
	Metadata *_metadata = nullptr;

	static SavedSpace _defaultSpace;
};

#endif
