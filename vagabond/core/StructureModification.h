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
#include <vagabond/core/Residue.h>
#include <vagabond/core/Sampler.h>

class Molecule;
class BondCalculator;
class ConcertedBasis;
class AtomContent;
class Atom;

class StructureModification
{
public:
	StructureModification(Molecule *mol, int num, int dims);
	
	void supplyTorsions(const std::vector<ResidueTorsion> &list,
                        const std::vector<float> &values);

	virtual ~StructureModification();

	void makeCalculator(Atom *anchor, bool has_mol);
	void startCalculator();
	
	void changeMolecule(Molecule *m);
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol = true) {};

	void fillBasis(ConcertedBasis *cb, const std::vector<ResidueTorsion> &list,
	               const std::vector<float> &values, int axis = 0);
	void checkMissingBonds(ConcertedBasis *cb);
	void addToHetatmCalculator(Atom *anchor);
	void finishHetatmCalculator();
	bool checkForMolecule(AtomGroup *grp);
	Molecule *_molecule = nullptr;

	BondCalculator *_hetatmCalc = nullptr;
	std::vector<BondCalculator *> _calculators;
	AtomGroup *_fullAtoms = nullptr;
	Sampler _sampler;
	
	struct ResidueTorsionList
	{
		const std::vector<ResidueTorsion> list;
		const std::vector<float> values;
	};
	
	std::vector<ResidueTorsionList> _torsionLists;

	int _num = 1;
	int _dims = 1;
	int _threads = 1;
	int _axis = 0;

	int _sideMissing = 0;
	int _mainMissing = 0;
	Residue *_unusedId{};
	
	BondCalculator::PipelineType _pType = BondCalculator::PipelineAtomPositions;
};

#endif
