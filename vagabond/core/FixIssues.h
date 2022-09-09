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

#ifndef __vagabond__FixIssues__
#define __vagabond__FixIssues__

#include <vector>
#include "Molecule.h"
#include "TorsionRef.h"
#include "Residue.h"

class Atom;
class Model;
class Residue;
class Molecule;
class Sequence;

typedef std::vector<Atom *> AtomVector;
typedef std::vector<TorsionRef> TorsionRefVec;
typedef std::pair<TorsionRef, TorsionRef> TorsionRefPair;
typedef std::vector<TorsionRefPair> TorsionRefPairs;

class FixIssues : public HasResponder<Responder<FixIssues> >
{
public:
	FixIssues(Molecule *m);

	void setModels(const std::vector<Model *> &models)
	{
		_models = models;
	}
	
	enum Options
	{
		FixNone = 0 << 0,
		FixPhenylalanine = 1 << 0,
		FixTyrosine = 1 << 1,
	};
	
	void setOptions(Options opts)
	{
		_options = opts;
	}
	
	void findIssues();
	
	size_t issueCount()
	{
		return _issues.size();
	}
	
	void fixIssue(int i);
	
	bool done()
	{
		return _done;
	}
private:
	void processModel(Model *m);

	AtomVector findAtoms(Molecule *m, std::string code, std::string atom);
	void processAtoms(Molecule *mol, AtomVector &atoms, float degree_diff);

	TorsionRefPairs findMatchingTorsions(const Atom *a, Residue *ref,
	                                     Residue *local);
	void checkTorsions(Molecule *mol, Residue *local, TorsionRefPairs &trps, 
	                   float expected_diff);

	std::vector<Model *> _models;
	Options _options = FixNone;
	bool _done = false;
	
	struct Issue
	{
		Molecule *mol; 
		Residue *local;
		TorsionRef torsion;
		float change;
		std::string message;
		
		std::string fullMessage()
		{
			std::string str = mol->id() + ", ";
			str += local->desc() + ": " + message;
			if (torsion.desc().length())
			{
				str += " for " + torsion.desc();
			}

			return str;
		}
	};
	
	void addIssue(Molecule *mol, Residue *local, TorsionRef tref, float change,
	              std::string message);
	std::vector<Issue> _issues;
	
	Molecule *_reference = nullptr;
	
public:
	std::string issueMessage(int i)
	{
		return _issues[i].fullMessage();
	}
};

#endif
