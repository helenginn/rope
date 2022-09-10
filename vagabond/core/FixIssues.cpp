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

#include "FixIssues.h"
#include "Model.h"
#include "AtomGroup.h"
#include "Sequence.h"
#include "Residue.h"
#include <unordered_map>

FixIssues::FixIssues(Molecule *m)
{
	_reference = m;
	_reference->sequence()->remapFromMaster(m->entity());

}

void FixIssues::findIssues()
{
	if (_options == FixNone)
	{
		return; // no issues found
	}
	
	std::cout << "Finding issues" << std::endl;
	_reference->model()->load();

	for (Model *m : _models)
	{
		processModel(m);
	}
	
	_done = true;
}

void FixIssues::processModel(Model *m)
{
	m->load();
	
	for (Molecule &m : m->molecules())
	{
		if (m.entity_id() != _reference->entity_id() || !m.isRefined())
		{
			continue;
		}
		
		if (&m == _reference)
		{
			continue;
		}

		Sequence *seq = m.sequence();
		if (_options & FixPhenylalanine)
		{
			AtomVector phes = findAtoms(&m, "PHE", "CD1");
			processAtoms(&m, phes, 180);
		}
		if (_options & FixTyrosine)
		{
			AtomVector tyrs = findAtoms(&m, "TYR", "CD1");
			processAtoms(&m, tyrs, 180);
		}
		if (_options & FixGlutamate)
		{
			AtomVector glus = findAtoms(&m, "GLU", "OE1");
			processAtoms(&m, glus, 180);
		}
		if (_options & FixAspartate)
		{
			AtomVector asps = findAtoms(&m, "ASP", "OD1");
			processAtoms(&m, asps, 180);
		}
		if (_options & FixArginine)
		{
			AtomVector args = findAtoms(&m, "ARG", "NH1");
			processAtoms(&m, args, 180);
		}
		if (_options & FixGlutamine)
		{
			AtomVector glns = findAtoms(&m, "GLU", "OE1");
			processAtoms(&m, glns, 180);
		}
		if (_options & FixAsparagine)
		{
			AtomVector asns = findAtoms(&m, "ASN", "OD1");
			processAtoms(&m, asns, 180);
		}
	}

	m->unload();
}

AtomVector FixIssues::findAtoms(Molecule *m, std::string code, std::string atom)
{
	AtomGroup *grp = m->currentAtoms();

	AtomVector final_list;
	AtomVector atoms = grp->atomsWithName(atom);
	
	for (Atom *a : atoms)
	{
		if (a->code() != code)
		{
			continue;
		}
		
		final_list.push_back(a);
	}
	
	return final_list;
}

TorsionRefPairs FixIssues::findMatchingTorsions(const Atom *a, Residue *ref,
                                                Residue *local)
{
	TorsionRefVec left, right;
	for (size_t i = 0; i < a->terminalTorsionCount(); i++)
	{
		BondTorsion *tt = a->terminalTorsion(i);
		std::string tmp = tt->desc();
		TorsionRef copyRef = ref->copyTorsionRef(tmp);
		TorsionRef copyLocal = local->copyTorsionRef(tmp);

		if (copyLocal.valid())
		{
			left.push_back(copyLocal);
		}
		if (copyRef.valid())
		{
			right.push_back(copyRef);
		}
	}

	TorsionRefPairs map;
	for (const TorsionRef &l : left)
	{
		for (const TorsionRef &r : right)
		{
			if (l.desc() == r.desc())
			{
				map.push_back(TorsionRefPair(l, r));
			}
		}
	}
	
	return map;
}

void FixIssues::checkTorsions(Molecule *mol, Residue *local, 
                              TorsionRefPairs &trps, float expected_diff)
{
	for (TorsionRefPair &pair : trps)
	{
		float local_angle = pair.first.refinedAngle();
		float ref_angle = pair.second.refinedAngle();
		
		float diff = ref_angle - local_angle;
		
		float change = 0;
		while (diff < -90 / 2)
		{
			change += expected_diff;
			diff += expected_diff;
		}
		while (diff >= +90 / 2)
		{
			change -= expected_diff;
			diff -= expected_diff;
		}
		
		if (fabs(change) > 1)
		{
			addIssue(mol, local, pair.first, change, "change by " + 
			         f_to_str(change, 0) + " degrees");
		}
	}

}

void FixIssues::processAtoms(Molecule *mol, AtomVector &atoms, float degree_diff)
{
	Sequence *seqref = _reference->sequence();
	Sequence *seq = mol->sequence();
	seq->remapFromMaster(mol->entity());

	for (const Atom *a : atoms)
	{
		int ttc = a->terminalTorsionCount();
		if (ttc < 1)
		{
			std::cout << "Atom doesn't have any terminal torsions" << std::endl;
			continue;
		}

		ResidueId id = a->residueId();
		// get the local residue
		Residue *local = seq->residue(id);
		
		if (local == nullptr)
		{
			continue;
		}

		// turn this into the master residue
		Residue *master = seq->master_residue(local);
		
		if (master == nullptr)
		{
			addIssue(mol, local, TorsionRef{}, 0, "missing residue in entity");
			continue;
		}

		// convert back to the local residue for the reference model
		Residue *ref = seqref->local_residue(master);
		
		if (ref == nullptr)
		{
			addIssue(mol, local, TorsionRef{}, 0, "missing residue in reference");
			continue;
		}
		
		// not tryptophan
		TorsionRefPairs trps = findMatchingTorsions(a, ref, local);
		checkTorsions(mol, local, trps, degree_diff);
	}
}

void FixIssues::addIssue(Molecule *mol, Residue *local, TorsionRef tref, 
                         float change, std::string message)
{
	Issue issue{mol, local, tref, change, message};
	_issues.push_back(issue);
	triggerResponse();
}

void FixIssues::fixIssue(int i)
{
	Issue &issue = _issues[i];
	
	if (issue.torsion.desc().length() > 0)
	{
		float angle = issue.torsion.refinedAngle();
		angle -= issue.change;
		issue.torsion.setRefinedAngle(angle);
		
		issue.local->replaceTorsionRef(issue.torsion);
	}
}
