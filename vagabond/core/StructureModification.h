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

#ifndef __vagabond__StructureModification__
#define __vagabond__StructureModification__

/** \class StructureModification will set up a calculator for editing a
 * Molecule in a structure */

#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/Sampler.h>

class Molecule;
class BondCalculator;
class ConcertedBasis;
class AtomContent;
class Atom;

class StructureModification
{
public:
	StructureModification(Molecule *mol);
	virtual ~StructureModification();

	void makeCalculator(Atom *anchor);
	void startCalculator();
	
	virtual void fillBasis(ConcertedBasis *cb) = 0;
protected:
	void addToHetatmCalculator(Atom *anchor);
	void finishHetatmCalculator();
	Molecule *_molecule = nullptr;

	BondCalculator *_hetatmCalc = nullptr;
	std::vector<BondCalculator *> _calculators;
	AtomContent *_fullAtoms = nullptr;
	Sampler _sampler;
	int _num = 0;
};

#endif
