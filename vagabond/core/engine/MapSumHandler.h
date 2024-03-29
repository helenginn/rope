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

#ifndef __vagabond__MapSumHandler__
#define __vagabond__MapSumHandler__

#include "engine/Handler.h"
#include "engine/ElementTypes.h"

class MiniJobMap;
class ElementSegment;
class AtomSegment;
class AtomMap;

class MapSumHandler : public Handler
{
public:
	/* map count should not be less than number of concurrent threads */
	MapSumHandler(int mapCount, const ElementSegment *templ);
	~MapSumHandler();
	
	void setMapCount(int maps)
	{
		_mapCount = maps;
	}
	
	size_t segmentCount()
	{
		return _mapPool.objectCount();
	}
	
	const AtomMap *templateMap()
	{
		return _template;
	}

	AtomSegment *acquireAtomSegmentIfAvailable();
	void returnSegment(AtomSegment *segment);
	void grab_map(std::map<std::string, GetEle> &gets,
	              Task<Result, void *> *submit = nullptr,
	              Task<SegmentAddition, AtomMap *> **atom_map = nullptr);

	void setup();

	void finish();
private:
	void createSegments();

	Pool<AtomSegment *> _mapPool;

	const ElementSegment *_segment = nullptr;
	
	AtomMap *_template = nullptr;
	int _mapCount = 1;
};

#endif
