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
#include <vagabond/gui/elements/DragResponder.h>

class Slider;
class Residue;
class Molecule;
class AtomContent;
class ConcertedBasis;

class AxisExplorer : public Display, public DragResponder, 
public StructureModification
{
public:
	AxisExplorer(Scene *prev, Molecule *mol, 
	             const std::vector<ResidueTorsion> &list,
	             const std::vector<Angular> &values);
	
	~AxisExplorer();

	virtual void setup();
	virtual void tieButton() {};

	void setupSlider();
	void submitJob(float prop);

	virtual void finishedDragging(std::string tag, double x, double y);
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol = true);
private:
	Slider *_rangeSlider = nullptr;

	void reportMissing();
	void setupColours();
	void setupColourLegend();
	
	double _min = -1; 
	double _max = 1; 
	double _step = 0.001;
	float _maxTorsion = 0;

	std::vector<ResidueTorsion> _list;
	std::vector<Angular> _values;
	
	int _dims = 1;
};

#endif
