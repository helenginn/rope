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

#ifndef __vagabond__ObjectGroup__
#define __vagabond__ObjectGroup__

#include <vector>
#include <string>
#include <vagabond/c4x/Cluster.h>

class HasMetadata;

class ObjectGroup
{
public:
	ObjectGroup() {};
	virtual ~ObjectGroup() {};

	std::vector<float> numbersForKey(std::string key);
	virtual void setSeparateAverage(std::vector<HasMetadata *> list);

	virtual const size_t headerCount() const = 0;
	virtual const int groupCount() const = 0;
	virtual void clearAverages() = 0;

	virtual void setWhiteList(std::vector<HasMetadata *> list) = 0;

	const size_t objectCount() const
	{
		return _objects.size();
	}
	
	const int indexOfObject(HasMetadata *obj) const;
	
	HasMetadata *const object(int i) 
	{
		return _objects[i];
	}
	
	const std::vector<HasMetadata *> &objects() const
	{
		return _objects;
	}
protected:
	std::vector<HasMetadata *> _objects;

};

#endif
