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

#include "SequenceComparison.h"
#include "AtomsFromSequence.h"
#include "Sequence.h"
#include "Residue.h"
#include "Grapher.h"
#include "Entity.h"
#include "Atom.h"
#include <sstream>
#include <gemmi/polyheur.hpp>

Sequence::Sequence()
{

}

Sequence::~Sequence()
{
	/* swap may look weird, but it removes memory footprint */
	std::list<Residue>().swap(_residues);
	std::list<Residue>().swap(_master);

	ResidueMap().swap(_map2Master);
	ResidueMap().swap(_map2Local);
}

Sequence::Sequence(const Sequence &seq) : IndexedSequence(seq)
{
	_residues = seq._residues;
	_master = seq._master;
	_anchor = seq._anchor;
	_entity = seq._entity;
	
	housekeeping();
	SequenceComparison *sc = newComparison(_entity);
	mapFromMaster(sc);
	delete sc;
}

Sequence::Sequence(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		char ch = str[i];

		std::string tlc = gemmi::expand_protein_one_letter(ch);
		Residue r{ResidueId(i + 1), tlc, " "};
		
		*this += r;
	}
}

Sequence::Sequence(Atom *anchor)
{
	_anchor = anchor;

	if (_anchor == nullptr)
	{
		return;
	}

	findSequence();
}

Sequence &Sequence::operator+=(Sequence *other)
{
	for (Residue &r : other->_residues)
	{
		*this += r;
	}

	for (Residue &r : other->_master)
	{
		_master.push_back(r);
	}

	return *this;
}

Sequence &Sequence::operator+=(Residue &res)
{
	res.setSequence(this);
	_residues.push_back(res);
	_id2Residue[res.id()] = &_residues.back();
	return *this;
}

void Sequence::findSequence()
{
	Grapher gr;
	gr.setSingleChain(true);
	AnchorExtension ext(_anchor, UINT_MAX);
	gr.generateGraphs(ext);

	int max = gr.graphCount();
	
	if (max <= 0)
	{
		return;
	}

	AtomGraph *last = gr.graph(0);
	
	while (last != nullptr)
	{
		Atom *ref = last->atom;
		ResidueId id = ref->residueId();
		std::string code = ref->code();
		std::string chain = ref->chain();
		Residue res{id, code, chain};
		_residues.push_back(res);
		
		last = gr.firstGraphNextResidue(last);
	}
	
	housekeeping();
}

void Sequence::housekeeping()
{
	_id2Residue.clear();
	for (Residue &r : _residues)
	{
		r.setSequence(this);
		const ResidueId &id = r.id();
		_id2Residue[id] = &r;
	}
}

std::string Sequence::str()
{
	std::ostringstream ss;
	std::vector<std::string> resvec;
	
	for (Residue &r : _residues)
	{
		resvec.push_back(r.code());
	}

	std::string olc = gemmi::one_letter_code(resvec);
//	std::replace(olc.begin(), olc.end(), 'X', ' ');
	
	return olc;
}

SequenceComparison *Sequence::newComparison(Entity *entity)
{
	if (entity == nullptr && _entity != nullptr)
	{
		entity = _entity;
	}
	else if (_entity == nullptr && entity != nullptr)
	{
		_entity = entity;
	}
	else if (_entity == nullptr && entity == nullptr)
	{
		return nullptr;
	}

	Sequence *master = entity->sequence();
	SequenceComparison *sc = new SequenceComparison(master, this);

	return sc;
}

void Sequence::mapFromMaster(SequenceComparison *sc)
{
	_map2Master.clear();
	_map2Local.clear();
	
	if (sc == nullptr)
	{
		return;
	}

	for (size_t i = 0; i < sc->entryCount(); i++)
	{
		if (sc->hasResidue(0, i) && sc->hasResidue(2, i))
		{
			Residue *theirs = sc->residue(0, i);
			Residue *mine = sc->residue(2, i);

			_map2Master[mine] = theirs;
			_map2Local[theirs] = mine;
		}
	}
	
	_master.clear();

	for (Residue &r : _residues)
	{
		if (_map2Master.count(&r))
		{
			_master.push_back(*_map2Master[&r]);
		}
		else
		{
			Residue res;
			res.setNothing(true);
			_master.push_back(res);
		}
	}
}

size_t Sequence::modelledResidueCount() const
{
	size_t count = 0;
	for (const Residue &r : _residues)
	{
		if (!r.nothing())
		{
			count++;
		}
	}

	return count;
}

void Sequence::addBufferResidue()
{
	Residue res;
	res.setNothing(true);

	_residues.push_back(res);
	_master.push_back(res);
}

Residue *const Sequence::local_residue(Residue *const master) const
{
	if (_map2Local.count(master) == 0)
	{
		return nullptr;
	}

	return _map2Local.at(master);
}

Residue *Sequence::master_residue(Residue *const local) const
{
	if (_map2Master.count(local) == 0)
	{
		return nullptr;
	}

	return _map2Master.at(local);
}

void Sequence::remapFromMaster(Entity *entity)
{
	if (entity == nullptr || _residues.size() == 0)
	{
		return;
	}

	std::list<Residue>::iterator local;
	local = _residues.begin();
	_entity = entity;
	_map2Master.clear();
	_map2Local.clear();

	for (Residue &mres : _master)
	{
		if (mres.nothing())
		{
			local++;
			continue;
		}

		const ResidueId &temp = mres.id();
		Residue *other = entity->sequence()->residueLike(temp);
		
		if (other)
		{
			Residue &lres = *local;
			_map2Master[&lres] = other;
			_map2Local[other] = &lres;
		}

		local++;
	}
}

Residue *Sequence::residueLike(const ResidueId &other) 
{
	for (Residue &r : _residues)
	{
		if (r.id() == other)
		{
			return &r;
		}
	}

	return nullptr;
}

const size_t Sequence::torsionCount() const
{
	size_t sum = 0;
	for (const Residue &r : _residues)
	{
		sum += r.torsionCount();
	}

	return sum;
}

void Sequence::addAtomPositionHeaders(std::vector<Atom3DPosition> &headers)
{
	for (Residue &residue : _residues)
	{
		if (residue.atomNames().size() == 0)
		{
			residue.housekeeping();
		}

		for (const std::string &name : residue.atomNames())
		{
			Atom3DPosition ap{};
			ap.atomName = name;
			ap.residue = &residue;
			ap.entity = _entity;
			headers.push_back(ap);
		}
	}
}

void Sequence::addResidueTorsions(std::vector<ResidueTorsion> &headers)
{
	for (Residue &residue : _residues)
	{
		for (const TorsionRef &torsion : residue.torsions())
		{
			ResidueTorsion rt{};
			rt.setTorsion(torsion);
			rt.setResidue(&residue);
			rt.setEntity(_entity);
			rt.housekeeping();
			headers.push_back(rt);
		}
	}

}

bool Sequence::torsionByName(const std::string name, Residue **res)
{
	if (name.rfind("t", 0) == std::string::npos || name.size() <= 1)
	{
		return false;
	}

	const char *numptr = &name[1];
	int num = atoi(numptr);
	
	*res = residueLike(ResidueId(num));
	
	return true;
}

/* called on the master sequence */
void Sequence::torsionsFromMapped(Sequence *seq, std::vector<Angular> &vals,
                                  rope::TorsionType type)
{
	for (Residue &master : _residues)
	{
		Residue *const local = seq->local_residue(&master);

		for (const TorsionRef &torsion : master.torsions())
		{
			if (!local)
			{
				vals.push_back(NAN);
				continue;
			}

			TorsionRef match = local->copyTorsionRef(torsion.desc());
			Angular f = NAN;
			
			if (match.valid())
			{
				switch (type)
				{
					case rope::RefinedTorsions:
					f = match.refinedAngle();
					break;

					case rope::TemporaryTorsions:
					f = match.tmpAngle();
					break;
					
					default:
					f = match.refinedAngle();
					break;
				}

				if (!match.coversMainChain())
				{
//					f = NAN;
				}
			}
			
			f.hyper = (match.isHyperParameter());

			vals.push_back(f);
		}
	}
}

AtomGroup *Sequence::convertToAtoms()
{
	AtomsFromSequence afs(*this);
	AtomGroup *grp = afs.atoms();
	return grp;
}

Sequence Sequence::fragment(ResidueId central, int buffer)
{
	Residue *centre = residueLike(central);
	std::list<Residue>::iterator start = iterator(*centre);
	std::list<Residue>::iterator finish = start;
	finish++;
	
	if (start == residues().end() || finish == residues().end())
	{
		return nullptr;
	}
	
	for (size_t i = 0; i < buffer; i++)
	{
		start--;
		finish++;
		if (start == residues().begin() ||
		    finish == residues().end())
		{
			return nullptr;
		}
		
		if (start->nothing() || finish->nothing())
		{
			return nullptr;
		}
	}
	
	Sequence fragment;
	for (std::list<Residue>::iterator it = start; it != finish; it++)
	{
		fragment += *it;
	}

	return fragment;
}
