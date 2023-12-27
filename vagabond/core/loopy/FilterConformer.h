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

#ifndef __vagabond__FilterConformer__
#define __vagabond__FilterConformer__

#include "StructureModification.h"

class StructureModification;
struct AtomBlock;
class Conformer;
class Loopy;

class FilterConformer
{
public:
	FilterConformer(Loopy *loopy);
	virtual ~FilterConformer() {};

	typedef std::function<float(const std::vector<AtomBlock> &)> ClashFunction;

	virtual bool acceptable(const float &value, const float &threshold) const = 0;
	
	bool operator()(Conformer *conf, float threshold) const;
	int submitJob(const std::vector<float> &vals) const;
protected:
	StructureModification::Resources &_resources;
	Loopy *const _loopy = nullptr;
	ClashFunction _clash{};
	
	std::string _tag = "filter";
	std::string _message;
};

#endif
