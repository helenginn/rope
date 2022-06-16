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

Sequence &Sequence::operator+=(Sequence *&other)
{
	for (Residue &r : other->_residues)
	{
		*this += r;
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
	gr.generateGraphs(_anchor);

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

Residue *const Sequence::local_residue(Residue *const master) const
{
	if (_map2Local.count(master) == 0)
	{
		return nullptr;
	}

	return _map2Local.at(master);
}

Residue *Sequence::master_residue(Residue *local) const
{
	if (_map2Master.count(local) == 0)
	{
		return nullptr;
	}

	return _map2Master.at(local);
}

void Sequence::remapFromMaster(Entity *entity)
{
	if (_map2Master.size() > 0 && _map2Local.size() > 0)
	{
//		return;
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

const size_t Sequence::torsionCount(bool onlyMain) const
{
	size_t sum = 0;
	for (const Residue &r : _residues)
	{
		sum += r.torsionCount(onlyMain);
	}

	return sum;
}

void Sequence::addTorsionNames(std::vector<std::string> &names, bool onlyMain)
{
	for (Residue &residue : _residues)
	{
		for (const TorsionRef &torsion : residue.torsions())
		{
			if (onlyMain && !torsion.isMain())
			{
				continue;
			}

			std::string id = "t" + residue.id().as_string() + ":" + torsion.desc();
			names.push_back(id);
		}
	}

}

void Sequence::torsionsFromMapped(Sequence *seq, std::vector<float> &vals,
                                  bool onlyMain, float dummy)
{
	for (Residue &master : _residues)
	{
		Residue *const local = seq->local_residue(&master);

		if (dummy >= 0)
		{
			vals.push_back(dummy);
		}
		
		for (const TorsionRef &torsion : master.torsions())
		{
			if (onlyMain && !torsion.isMain())
			{
				continue;
			}
			
			if (!local)
			{
				vals.push_back(NAN);
				continue;
			}

			TorsionRef match = local->copyTorsionRef(torsion.desc());
			float f = NAN;
			
			if (match.valid())
			{
				f = match.refinedAngle();
			}

			vals.push_back(f);
		}
	}
}
