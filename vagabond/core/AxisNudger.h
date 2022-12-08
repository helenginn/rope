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

#ifndef __vagabond__AxisNudger__
#define __vagabond__AxisNudger__

#include "RefinementInfo.h"
#include "PlausibleRoute.h"

class AxisNudger : public PlausibleRoute
{
public:
	AxisNudger(Refine::Info *info, int axis,
	           Cluster<MetadataGroup> *cluster);

	virtual void setup();
	void run();
	void addToInfo();
private:
	BondTorsion *otherTorsion(AtomGraph *ag);
	void collectTorsionPairs();
	float scoreForPoint(Point &point);
	void nudge();
	
	struct RamachandranPairs
	{
		BondTorsion *first;
		BondTorsion *second;
	};
	
	std::vector<RamachandranPairs> _pairs;

	Refine::Info *_info = nullptr;
	int _idx = -1;
};

#endif
