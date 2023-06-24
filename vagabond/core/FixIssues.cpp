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

FixIssues::FixIssues(Polymer *m)
{
	_reference = m;
	_reference->sequence()->remapFromMaster(m->entity());

}

FixIssues::~FixIssues()
{

}

void FixIssues::findIssues()
{
	if (_options == FixNone)
	{
		return; // no issues found
	}
	
	_reference->model()->load();
	std::cout << "Ref molecule: " << _reference->id() << std::endl;

	for (Model *m : _models)
	{
		processModel(m);
		
		if (_finish)
		{
			break;
		}
	}
	
	_reference->model()->unload();
	_done = true;
	triggerResponse();
}

void FixIssues::stop()
{
	_finish = true;
}

void FixIssues::processModel(Model *m)
{
	m->load();
	
	for (Polymer &m : m->polymers())
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
		if (_options & Fix360Rotations)
		{
			AtomVector all = m.currentAtoms()->atomVector();
			processAtoms(&m, all, 360);
		}
		if (_options & FixPhenylalanine)
		{
			AtomVector phes = findAtoms(&m, "PHE", "CD1");
			processAtoms(&m, phes, 180);
			phes = findAtoms(&m, "PHE", "CD2");
			processAtoms(&m, phes, 180);
		}
		if (_options & FixTyrosine)
		{
			AtomVector tyrs = findAtoms(&m, "TYR", "CD1");
			processAtoms(&m, tyrs, 180);
			tyrs = findAtoms(&m, "TYR", "CD2");
			processAtoms(&m, tyrs, 180);
		}
		if (_options & FixGlutamate)
		{
			AtomVector glus = findAtoms(&m, "GLU", "OE1");
			processAtoms(&m, glus, 180);
			glus = findAtoms(&m, "GLU", "OE2");
			processAtoms(&m, glus, 180);
		}
		if (_options & FixAspartate)
		{
			AtomVector asps = findAtoms(&m, "ASP", "OD1");
			processAtoms(&m, asps, 180);
			asps = findAtoms(&m, "ASP", "OE1");
			processAtoms(&m, asps, 180);
		}
		if (_options & FixArginine)
		{
			AtomVector args = findAtoms(&m, "ARG", "NH1");
			processAtoms(&m, args, 180);
			args = findAtoms(&m, "ARG", "NH2");
			processAtoms(&m, args, 180);
		}
		if (_options & FixGlutamine)
		{
			AtomVector glns = findAtoms(&m, "GLU", "OE1");
			processAtoms(&m, glns, 180);
			glns = findAtoms(&m, "GLU", "NE2");
			processAtoms(&m, glns, 180);
		}
		if (_options & FixHistidine)
		{
			AtomVector hiss = findAtoms(&m, "HIS", "ND1");
			processAtoms(&m, hiss, 180);
			hiss = findAtoms(&m, "HIS", "CD2");
			processAtoms(&m, hiss, 180);
		}
		if (_options & FixAsparagine)
		{
			AtomVector asns = findAtoms(&m, "ASN", "OD1");
			processAtoms(&m, asns, 180);
			asns = findAtoms(&m, "ASN", "ND2");
			processAtoms(&m, asns, 180);
		}
		if (_options & FixPeptideFlips)
		{
			fixPeptideFlips(&m);
		}
	}

	m->unload();
}

void check_phi_psi(float phi, float psi, float &dphi, float &dpsi)
{
	if (fabs(phi + psi) > 180)
	{
		while (phi < -90)
		{
			phi += 360; dphi += 360;
			if (fabs(phi + psi) < 180) { return; }
		}
		while (phi >= +90)
		{
			phi -= 360; dphi -= 360;
			if (fabs(phi + psi) < 180) { return; }
		}
		
		while (psi < -90)
		{
			psi += 360; dpsi += 360;
			if (fabs(phi + psi) < 180) { return; }
		}
		while (psi >= +90)
		{
			psi -= 360; dpsi -= 360;
			if (fabs(phi + psi) < 180) { return; }
		}
	}
}

void FixIssues::fixPeptideFlips(Polymer *m)
{
	AtomVector cas = findAtoms(m, "", "CA");

	for (Atom *ca : cas)
	{
		Atom *c = nullptr;
		Atom *n = nullptr;
		for (int i = 0; i < ca->bondLengthCount(); i++)
		{
			if (ca->connectedAtom(i)->atomName() == "C")
			{
				c = ca->connectedAtom(i);
				break;
			}
		}
		
		if (!c) continue;
		for (int i = 0; i < c->bondLengthCount(); i++)
		{
			if (c->connectedAtom(i)->atomName() == "N")
			{
				n = c->connectedAtom(i);
				break;
			}
		}
		if (!n) continue;
		
		Residue *local_c = nullptr; Residue *ref_c = nullptr;
		residuesForAtom(m, c, local_c, ref_c);

		Residue *local_n = nullptr; Residue *ref_n = nullptr;
		residuesForAtom(m, n, local_n, ref_n);
		
		if (!local_c || !ref_c || !local_n || !ref_n)
		{
			continue;
		}
		
		TorsionRefPairs c_trps = findMatchingTorsions(c, local_c, ref_c);
		TorsionRefPairs n_trps = findMatchingTorsions(n, local_n, ref_n);
		
		TorsionRef *phi_ref = nullptr;
		TorsionRef *psi_ref = nullptr;
		TorsionRef *phi_local = nullptr;
		TorsionRef *psi_local = nullptr;
		
		for (TorsionRefPair &pair : c_trps)
		{
			if (pair.first.atomName(0) == "C" && 
			    pair.first.atomName(3) == "C")
			{
				phi_ref = &(pair.first);
				phi_local = &(pair.second);
			}
		}
		
		for (TorsionRefPair &pair : n_trps)
		{
			if (pair.first.atomName(0) == "N" && 
			    pair.first.atomName(3) == "N")
			{
				psi_ref = &(pair.first);
				psi_local = &(pair.second);
			}
		}
		
		if (!phi_ref || !psi_ref || !phi_local || !psi_local)
		{
			continue;
		}

		float phi_diff = phi_ref->refinedAngle() - phi_local->refinedAngle();
		float psi_diff = psi_ref->refinedAngle() - psi_local->refinedAngle();
		float dphi = 0; float dpsi = 0;
		
		check_phi_psi(phi_diff, psi_diff, dphi, dpsi);

		if (fabs(dphi) > 1)
		{
			addIssue(m, local_c, *phi_local, dphi, "peptide flip phi, change "\
			         "by " + f_to_str(dphi, 0) + " degrees");
		}
		if (fabs(dpsi) > 1)
		{
			addIssue(m, local_n, *psi_local, dpsi, "peptide flip psi, change "\
			         "by " + f_to_str(dpsi, 0) + " degrees");
		}
	}
}

AtomVector FixIssues::findAtoms(Polymer *m, std::string code, std::string atom)
{
	AtomGroup *grp = m->currentAtoms();

	AtomVector final_list;
	AtomVector atoms = grp->atomsWithName(atom);
	
	for (Atom *a : atoms)
	{
		if (a->code() != code && code.length() > 0)
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

void FixIssues::checkTorsions(Polymer *mol, Residue *local, 
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

void FixIssues::residuesForAtom(Polymer *mol, const Atom *a, 
                                Residue *&local, Residue *&ref)
{
	Sequence *seqref = _reference->sequence();
	Sequence *seq = mol->sequence();
	seq->remapFromMaster(mol->entity());
	seqref->remapFromMaster(mol->entity());

	int ttc = a->terminalTorsionCount();
	if (ttc < 1)
	{
		return;
	}

	ResidueId id = a->residueId();
	// get the local residue
	local = seq->residue(id);

	if (local == nullptr)
	{
		return;
	}

	// turn this into the master residue
	Residue *master = seq->master_residue(local);

	if (master == nullptr)
	{
//		addIssue(mol, local, TorsionRef{}, 0, "missing residue in entity");
		return;
	}

	// convert back to the local residue for the reference model
	ref = seqref->local_residue(master);

	if (ref == nullptr)
	{
		addIssue(mol, local, TorsionRef{}, 0, "missing residue in reference");
		return;
	}
}

void FixIssues::processAtoms(Polymer *mol, AtomVector &atoms, float degree_diff)
{
	for (const Atom *a : atoms)
	{
		Residue *local = nullptr;
		Residue *ref = nullptr;
		residuesForAtom(mol, a, local, ref);
		
		if (!local || !ref)
		{
			continue;
		}
		
		TorsionRefPairs trps = findMatchingTorsions(a, ref, local);
		checkTorsions(mol, local, trps, degree_diff);
	}
}

void FixIssues::addIssue(Polymer *mol, Residue *local, TorsionRef tref, 
                         float change, std::string message)
{
	Issue issue{mol, local, tref, change, message};
	
	std::vector<Issue>::iterator it;
	it = std::find(_issues.begin(), _issues.end(), issue);
	
	if (it != _issues.end()) 
	{
		*it = issue;
		triggerResponse();
		return;
	}

	_issues.push_back(issue);
	triggerResponse();
}

void FixIssues::fixIssue(int i)
{
	Issue &issue = _issues[i];
	
	if (issue.torsion.desc().length() > 0)
	{
		std::cout << issue.fullMessage() << std::endl;
		float angle = issue.torsion.refinedAngle();
		std::cout << "angle " << angle << " ";
		angle -= issue.change;
		std::cout << " to " << angle << std::endl;
		issue.torsion.setRefinedAngle(angle);
		
		issue.local->replaceTorsionRef(issue.torsion);
	}
}
