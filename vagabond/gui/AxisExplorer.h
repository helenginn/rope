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
#include <vagabond/core/StructureModification.h>
#include <vagabond/core/PositionSampler.h>
#include <vagabond/core/RAMovement.h>
#include <vagabond/gui/elements/DragResponder.h>

class Slider;
class Residue;
class Instance;
class AtomContent;
class ConcertedBasis;

class AxisExplorer : public Display, public DragResponder, 
public StructureModification, public PositionSampler
{
public:
	AxisExplorer(Scene *prev, Instance *mol, const RTAngles &angles);
	~AxisExplorer();

	virtual void setup();
	virtual void tieButton() {};

	void setupSlider();
	void submitJob(float prop);

	void adjustTorsions();

	virtual void buttonPressed(std::string tag, Button *button);
	virtual void finishedDragging(std::string tag, double x, double y);

	virtual bool prewarnAtoms(BondSequence *bc, const Coord::Get &get,
	                          Vec3s &positions);

	virtual void prewarnBonds(BondSequence *seq, const Coord::Get &get,
	                          Floats &torsions) {};
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol = true);
	void setupColoursForList(RTAngles &angles);
private:
	Slider *_rangeSlider = nullptr;

	void askForAtomMotions();
	void setupColours();
	void setupColourLegend();
	
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

	RAMovement _movement;
	
	int _dims = 1;
};

#endif
