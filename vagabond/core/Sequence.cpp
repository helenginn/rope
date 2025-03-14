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
#include "Atom3DPosition.h"
#include "AtomsFromSequence.h"
#include "EntityManager.h"
#include "Sequence.h"
#include "Residue.h"
#include "Grapher.h"
#include "Entity.h"
#include "Atom.h"
#include <sstream>
#include <gemmi/resinfo.hpp>

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
	_entity_id = seq._entity_id;
	
	housekeeping();
	
	/*
	if (_master.size() == 0)
	{
		SequenceComparison *sc = newComparison();
		mapFromMaster(sc);
		delete sc;
	}
	*/
}

Sequence::Sequence(std::string str, int offset)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		char ch = str[i];

		std::string tlc = gemmi::expand_one_letter(ch, gemmi::ResidueKind::AA);

		Residue r{ResidueId(i + 1 + offset), tlc, " "};
		
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
	gr.setInSequence(true);
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
	std::string olc;
	
	for (Residue &r : _residues)
	{
		resvec.push_back(r.code());
		gemmi::ResidueInfo info = gemmi::find_tabulated_residue(r.code());
		char ch = info.one_letter_code;
		if (info.one_letter_code == ' ')
		{
			ch = 'X';
		}
		olc += ch;
	}

//	std::replace(olc.begin(), olc.end(), 'X', ' ');
	
	return olc;
}

SequenceComparison *Sequence::newComparison()
{
	Sequence *master = entity()->sequence();
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

Residue *const Sequence::local_residue(Residue *const master) 
{
	if (_map2Local.size() == 0)
	{
		remapFromMaster();
	}

	if (!master || _map2Local.count(master) == 0)
	{
		return nullptr;
	}

	return _map2Local.at(master);
}

Residue *Sequence::master_residue(Residue *const local) 
{
	if (_map2Master.size() == 0)
	{
		remapFromMaster();
	}

	if (!local || _map2Master.count(local) == 0)
	{
		return nullptr;
	}

	return _map2Master.at(local);
}

void Sequence::setEntity(Entity *entity)
{
	_entity_id = entity->name();
	_entity = entity;
}

Entity *Sequence::entity()
{
	if (_entity)
	{
		return _entity;
	}

	if (_entity_id.length())
	{
		Entity *ent = EntityManager::manager()->entity(_entity_id);
		_entity = ent;
	}
	
	return _entity;
}

void Sequence::remapFromMaster()
{
	if (!entity() || _residues.size() == 0)
	{
		return;
	}
	
	//already done
	if (_map2Master.size() > 0 && _map2Local.size() > 0)
	{
		return;
	}

	std::list<Residue>::iterator local;
	local = _residues.begin();
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
		Residue *other = entity()->sequence()->residueLike(temp);
		
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

void Sequence::addAtomPositionHeaders(std::vector<Atom3DPosition> &headers,
                                      bool main_chain_only)
{
	for (Residue &residue : _residues)
	{
		if (residue.atomNames().size() == 0)
		{
			residue.housekeeping();
		}

		for (const std::string &name : residue.atomNames())
		{
			if (main_chain_only && !Atom::isMainChain(name))
			{
				continue;
			}

			Atom3DPosition ap{};
			ap.setMaster(&residue);
			ap.setAtomName(name);
			ap.setEntity(entity());
			headers.push_back(ap);
		}
	}
}

void Sequence::addResidueTorsions(std::vector<ResidueTorsion> &headers,
                                  bool as_master,
                                  bool add_hydrogens)
{
	for (Residue &residue : _residues)
	{
		for (const TorsionRef &torsion : residue.torsions())
		{
			if (!add_hydrogens && torsion.hasHydrogen())
			{
				continue;
			}

			ResidueTorsion rt{};
			rt.setTorsion(torsion);
			if (as_master)
			{
				rt.setMaster(&residue);
			}
			else
			{
				rt.setLocalId(residue.id());
			}
			rt.setEntity(entity());
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
void Sequence::torsionsFromMapped(RTAngles &angles, rope::TorsionType type)
{
	for (int i = 0; i < angles.size(); i++)
	{
		const ResidueTorsion &rt = angles.rt(i);

		Residue *const master = rt.master();
		const TorsionRef &tref = rt.torsion();

		Residue *const local = local_residue(master);
		
		if (!local)
		{
			continue;
		}

		TorsionRef match = local->copyTorsionRef(tref.desc());

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
		}

		f.hyper = (match.isHyperParameter());

		angles.storage(i) = f;
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
