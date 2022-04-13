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

#include "Handler.h"
#include "BondCalculator.h"
#include "BondSequence.h"

class AtomGroup;
class ElementSegment;

/**
 * \class MapTransferHandler
 * handles the transfer of atom positions from BondSequence to produce
 * electron distribution maps (density/electrostatic potential) for each
 * individual element. */

class MapTransferHandler : public Handler
{
public:
	MapTransferHandler(BondCalculator *calculator = nullptr);
	
	~MapTransferHandler();

	/**let the MTH know which atoms will be involved in the calculation.
	 * Atoms in @param all but not in @param sub will be included as part
	 * of the 'constant' segment. 
	 * @param all every atom to be considered in the analysis
	 * @param sub sub-group of atoms which will change during analysis */
	void supplyAtomGroup(std::vector<Atom *> all, 
	                     std::vector<Atom *> sub);

	/**prepares MapTransfers and appropriate thread pools etc. 
	 * @param elements map connecting element symbol e.g. Ca to number of
	 * instances of atom */
	void supplyElementList(std::map<std::string, int> elements);
	
	/** after supplying atom groups and element list, run setup() to
	 *  allocate internal resources */
	void setup();
	
	/** set length dimension of cubic voxel
	 * 	@param dim length in Angstroms */
	void setCubeDim(float dim)
	{
		_cubeDim = dim;
	}

	MiniJobMap *makeJobForElement(std::string ele,
	                           std::vector<BondSequence::ElePos> &epos);
	
	void setupMiniJobs(Job *job, std::vector<BondSequence::ElePos> &epos);
	ElementSegment *acquireSegment(std::string ele);
	void returnSegment(ElementSegment *segment);

	MiniJobMap *acquireMiniJob(std::string ele);

	void start();
	void finish();
	
	void setSumHandler(MapSumHandler *handler)
	{
		_sumHandler = handler;
	}
	
	const size_t segmentCount() const
	{
		return _segments.size();
	}
	
	const ElementSegment *segment(int i) const
	{
		return _segments[i];
	}

private:
	void allocateSegments();
	void findThreadCount();
	void prepareThreads();
	void finishThreads();
	void getRealDimensions(std::vector<Atom *> &sub);

	std::vector<Atom *> _all;
	std::vector<Atom *> _sub;

	std::vector<ElementSegment *> _segments;
	std::vector<std::string> _elements;
	std::map<std::string, ElementSegment *> _element2Segment;
	std::map<std::string, Pool<ElementSegment *> > _pools;

	std::map<std::string, Pool<MiniJobMap *> > _miniJobPools;
	std::mutex _handout;

	BondCalculator *_calculator = nullptr;
	MapSumHandler *_sumHandler = nullptr;
	
	float _cubeDim = 0.5;
	int _threads = 2;
	
	glm::vec3 _min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	glm::vec3 _max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	glm::vec3 _pad = glm::vec3(3, 3, 3);
};

#endif
