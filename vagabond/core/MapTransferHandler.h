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

class MapTransferHandler : public Handler
{
public:
	MapTransferHandler(BondCalculator *calculator = nullptr);

	/**let the MTH know which atoms will be involved in the calculation.
	 * Atoms in @param all but not in @param sub will be included as part
	 * of the 'constant' segment. 
	 * @param all every atom to be considered in the analysis
	 * @param sub sub-group of atoms which will change during analysis */
	void supplyAtomGroup(AtomGroup *all, AtomGroup *sub);

	/**prepares MapTransfers and appropriate thread pools etc. 
	 * @param elements map connecting element symbol e.g. Ca to number of
	 * instances of atom */
	void supplyElementList(std::map<std::string, int> elements);
	
	void setupMiniJob(std::vector<BondSequence::ElePos> &epos);

private:
	AtomGroup *_all = nullptr;
	AtomGroup *_sub = nullptr;
	std::map<std::string, int> _elements;
	BondCalculator *_calculator = nullptr;
};

#endif
