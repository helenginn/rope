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

#ifndef __vagabond__AxisExplorer__
#define __vagabond__AxisExplorer__

#include <vagabond/c4x/Angular.h>
#include <vagabond/gui/Display.h>
#include <vagabond/utils/Vec3s.h>
#include <vagabond/core/RAMovement.h>
#include <vagabond/core/StructureModification.h>
#include <vagabond/gui/elements/DragResponder.h>

class Slider;
class Residue;
class Instance;
class ClusterSVD;
class AtomContent;
class TabulatedData;
class ConcertedBasis;

class AxisExplorer : public Display, public DragResponder, 
public StructureModification
{
public:
	AxisExplorer(Scene *prev, Instance *mol, const RTAngles &angles);
	~AxisExplorer();

	virtual void setup();
	virtual void tieButton() {};

	void setCluster(ClusterSVD *const &cluster, TorsionData *const &data)
	{
		_cluster = cluster;
		_tData = data;
	}

	void setupSlider();
	void submitJob(float prop);

	void adjustTorsions();

	virtual void buttonPressed(std::string tag, Button *button);
	virtual void finishedDragging(std::string tag, double x, double y);
protected:
	void setupColoursForList(RTAngles &angles);
	virtual void prepareResources();
private:
	Slider *_rangeSlider = nullptr;

	void askForAtomMotions();
	void makeMenu();
	void setupColours();
	void setupColourLegend();
	void supplyTorsions(CoordManager *manager);
	
	struct mapping
	{
		int operator()(BondSequence *seq, int block)
		{
			if (map.count(seq) && map.at(seq).count(block))
			{
				return map[seq][block];
			}

			return -1;
		}
		
		void setSeqBlockIdx(BondSequence *seq, int block, int move_idx)
		{
			map[seq][block] = move_idx;
		}

		std::map<BondSequence *, std::map<int, int>> map;
	};
	
	mapping _mapping;
	
	double _min = -1; 
	double _max = 1; 
	double _step = 0.001;
	float _maxTorsion = 0;

	TabulatedData *_data = nullptr;
	ClusterSVD *_cluster = nullptr;
	TorsionData *_tData = nullptr;
	RTAngles _rawAngles;
	RAMovement _movement;
	
	int _dims = 1;
};

#endif
