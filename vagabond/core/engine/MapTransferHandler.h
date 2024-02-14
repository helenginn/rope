// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__MapTransferHandler__
#define __vagabond__MapTransferHandler__

#include "TransferHandler.h"
#include "BondSequence.h"

class AtomGroup;
class MapSumHandler;
class ElementSegment;
class PointStoreHandler;

/**
 * \class MapTransferHandler
 * handles the transfer of atom positions from BondSequence to produce
 * electron distribution maps (density/electrostatic potential) for each
 * individual element. */

class MapTransferHandler : public TransferHandler
{
public:
	MapTransferHandler(const std::map<std::string, int> &elements,
	                   int number_of_maps);
	
	~MapTransferHandler();

	/**prepares MapTransfers and appropriate thread pools etc. 
	 * @param elements map connecting element symbol e.g. Ca to number of
	 * instances of atom */
	void supplyElementList(std::map<std::string, int> elements);
	
	/** after supplying atom groups and element list, run setup() to
	 *  allocate internal resources */
	void setup();

	ElementSegment *acquireSegment(std::string ele);
	void returnSegment(ElementSegment *segment);
	
	void extract(std::map<std::string, GetEle> &eleTasks);
	
	const size_t elementCount() const
	{
		return _elements.size();
	}
	
	const std::vector<std::string> &elements() const
	{
		return _elements;
	}
	
	const ElementSegment *segment(int i) const
	{
		return _segments[i];
	}
private:
	void allocateSegments();
	ElementSegment *acquireSegmentIfAvailable(std::string ele);

	std::vector<ElementSegment *> _segments;
	std::vector<std::string> _elements;
	std::map<std::string, Pool<ElementSegment *> > _pools;
};

#endif
