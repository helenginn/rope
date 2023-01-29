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

#ifndef __vagabond__PathManager__
#define __vagabond__PathManager__

#include "Path.h"
#include "Manager.h"

class PathManager : public Manager<Path>
{
public:
	PathManager();

	virtual Path *insertIfUnique(Path &p);

	virtual const std::string progressName() const
	{
		return "paths";
	}
	
	std::vector<Path *> pathsForInstance(Instance *mol);

	void housekeeping();
	
	friend void to_json(json &j, const PathManager &value);
	friend void from_json(const json &j, PathManager &value);
private:

};

inline void to_json(json &j, const PathManager &value)
{
	j["paths"] = value._objects;
}

inline void from_json(const json &j, PathManager &value)
{
	try
	{
		std::list<Path> paths = j.at("paths");
		value._objects = paths;
	}
	catch (...)
	{

	}
	
	for (Path &p : value._objects)
	{
		p.housekeeping();
	}
}


#endif
