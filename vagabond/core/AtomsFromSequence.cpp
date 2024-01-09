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

#include "AtomsFromSequence.h"
#include "AtomGroup.h"
#include "Sequence.h"
#include "BondLength.h"
#include "Knotter.h"
#include "files/PdbFile.h"
#include "GeometryTable.h"

AtomsFromSequence::AtomsFromSequence(Sequence &seq, bool oxt) : _seq(seq)
{
	_oxt = oxt;

}

AtomsFromSequence::~AtomsFromSequence()
{

}

GeometryTable *AtomsFromSequence::geometry()
{
	GeometryTable &gt = GeometryTable::getAllGeometry();
	return &gt;
}

AtomGroup *AtomsFromSequence::atoms()
{
	GeometryTable *gt = geometry();
	gt->filterPeptides(true);
	AtomGroup *total = new AtomGroup();
	total->setOwns(true);

	int num = 0;
	Atom *last = nullptr;
	for (int i = 0; i < _seq.size(); i++)
	{
		Residue *r = _seq.residue(i);
		std::string code = r->code();
		
		if (code.length() != 3 || code == "UNK")
		{
			continue;
		}

		const ResidueId &id = r->id();

		int term = (i == _seq.size() - 1) ? 1 : (i == 0 ? -1 : 0);
		if (!_oxt) term = 0;
		AtomGroup *ag = gt->constructResidue(code, id, &num, term);
		
		if (ag->size() == 0)
		{
			continue;
		}
		
		Atom *next = ag->firstAtomWithName("N");
		total->add(ag);

		if (last != nullptr)
		{
			float standard = gt->length(code, last->atomName(),
			                            next->atomName(), true);
			new BondLength(total, next, last, standard);
		}

		last = ag->firstAtomWithName("C");

		delete ag;
	}
	
	Knotter knotter(total, gt);
	knotter.setDoLengths(false);
	knotter.knot();
	
	for (size_t i = 0; i < total->bondTorsionCount(); i++)
	{
		BondTorsion *t = total->bondTorsion(i);
		if (t->isConstrained())
		{
			continue;
		}
		
		t->setRefinedAngle(0.f);
		if (t->isPeptideBond())
		{
			t->setRefinedAngle(180.f);
		}

		if (!t->coversMainChain() && t->desc() == "N-CA-C-N")
		{
			t->setRefinedAngle(150.f);
		}
		if (!t->coversMainChain() && t->desc() == "C-N-CA-C")
		{
			t->setRefinedAngle(-60.f);
		}
	}
	
	total->recalculate();

	return total;
}
