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

#include "Write.h"
#include "Visual.h"
#include "Untangle.h"
#include "UntangleView.h"
#include <vagabond/core/GeometryTable.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondLength.h>

Untangle::Untangle(UntangleView *view, const std::string &filename,
const std::set<std::string> &geometries)
{
	_filename = filename;
	_view = view;
	_geometries = geometries;
	GeometryTable::loadExtraGeometries(_geometries);
	
	setup();
}

void Untangle::loadFile()
{
	PdbFile file(_filename);
	file.parse();
	_group = file.atoms();
}

void Untangle::extractBonds(Atom *const &atom)
{
	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		BondLength *bl = atom->bondLength(i);
		if (_map.count(bl) == 0)
		{
			_bonds.push_back(TangledBond(*bl));
			_map[bl] = &_bonds.back();
		}
	}
}

void Untangle::extract()
{
	for (Atom *const &atom : _group->atomVector())
	{
		extractBonds(atom);
	}
}

const std::vector<Atom *> &Untangle::atoms()
{
	return _group->atomVector();
}

void Untangle::setup()
{
	loadFile();
	extract();
	setupNonBonds();
}

void Untangle::setupNonBonds()
{
	_nonBonds = new NonBonds();
	_nonBonds->setAtoms(atoms());
}

void Untangle::informationAbout(Atom *atom)
{
	_nonBonds->evaluateAtom(atom, true);

}

float Untangle::reevaluateAtoms()
{
	float total = 0;
	_view->visual()->clearBadness();
	_nonBonds->setUpdateBadness(_view->visual()->updateBadness());

	int count = 0;
	for (Atom *atom : atoms())
	{
		total += _nonBonds->evaluateAtom(atom, false);
		count++;
	}

	_clash = total;
	return _clash;
}

void Untangle::addTouchedBond(BondLength *bl)
{
	if (std::find(_touched.begin(), _touched.end(), _map[bl])
	    == _touched.end())
	{
		_touched.push_back(_map[bl]);
	}
}

void Untangle::switchConfs(Atom *a, const std::string &l, 
                           const std::string &r, bool one_only, 
                           const std::function<void(Atom *)> &per_job)
{
	auto job = [l, r, &per_job](Atom *a) -> bool
	{
		try
		{
			Atom::AtomPlacement place = a->conformerPositions().at(l);
			a->conformerPositions()[l] = a->conformerPositions().at(r);
			a->conformerPositions()[r] = place;
			if (per_job)
			{
				per_job(a);
			}
			return true;
		}
		catch (...)
		{
			return false;
		}
	};
	
	auto doable_atom = [l, r](Atom *a) -> bool
	{
		return (a->conformerPositions().count(l) &&
		        a->conformerPositions().count(r));
	};

	Atom *orig = a;
	std::set<Atom *> really_done;

	auto job_from_bl = [&really_done, orig, job]
	(std::set<Atom *> &done, BondLength *bl) -> bool
	{
		bool success = false;
		for (int i = 0; i < 2; i++)
		{
			Atom *a = bl->atom(i);
			if (really_done.count(a) == 0 && a != orig)
			{
				really_done.insert(a);
				success |= job(a);
			}
		}
		return success;
	};

	Atom *found = nullptr; // get the correct direction to move forward in
	for (int i = 0; i < a->bondLengthCount(); i++)
	{
		if (a->connectedAtom(i)->atomNum() < a->atomNum() &&
		    a->connectedAtom(i)->residueNumber() >= a->residueNumber() - 1 &&
		    a->connectedAtom(i)->residueNumber() <= a->residueNumber())
		{
			found = a->connectedAtom(i);
			addTouchedBond(a->bondLength(i));
			break;
		}
	}

	bool first_success = job(a);

	if (!one_only && first_success)
	{
		if ((a->code() == "PRO" || a->code() == "TYR" ||
		     a->code() == "PHE" || a->code() == "TRP" ||
		     a->code() == "HIS") && !a->isMainChain())
		{
		}
		else
		{
			doJobDownstream(a, doable_atom, job_from_bl, INT_MAX, {found});
		}
	}
	
}

void Untangle::doJobDownstream(Atom *atom, const CheckAtom &check,
                               const DownstreamJob &job, 
                               int max_hops, std::set<Atom *> done)
{
	struct Entry
	{
		Atom *atom;
		BondLength *bl;
		int hops;
	};

	std::vector<Entry> queue;
	
	auto other_side = [&done](BondLength *bl) -> Atom *
	{
		if (done.count(bl->atom(0))) { return bl->atom(1); }
		if (done.count(bl->atom(1))) { return bl->atom(0); }
		return nullptr;
	};

	auto add_bl = [&queue, &done, &check](Atom *atom, int hops_remaining)
	{
		if (hops_remaining <= 0)
		{
			return;
		}

		for (int i = 0; i < atom->bondLengthCount(); i++)
		{
			Atom *next = atom->connectedAtom(i);
			if (done.count(next))
			{
				continue;
			}
			
			if (next->residueNumber() < atom->residueNumber() ||
			    next->residueNumber() > atom->residueNumber() + 1)
			{
				continue;
			}
			
			if (!check(next))
			{
				continue;
			}
			
			queue.push_back({atom, atom->bondLength(i), hops_remaining - 1});
		}
	};
	
	add_bl(atom, max_hops);

	bool success = true;
	while (queue.size())
	{
		Entry last = queue.back();

		Atom *atom = last.atom;
		int hops_remaining = last.hops;
		BondLength *bond = last.bl;
		
		queue.pop_back();

		success = job(done, bond);
		done.insert(atom);

		Atom *next = other_side(bond);
		add_bl(next, hops_remaining);
	}
}

std::vector<TangledBond *> Untangle::volatileBonds()
{
	std::vector<TangledBond *> tangles;
	
	for (TangledBond &tb : _bonds)
	{
		if (!tb.hasHydrogens())
		{
			tangles.push_back(&tb);
		}
	}

	std::sort(tangles.begin(), tangles.end(),
	[](TangledBond *const &a, TangledBond *const &b)
	{
		return a->total_score(1.) < b->total_score(1.);
	});
	
	size_t count = 0;
	for (TangledBond *const &tb : tangles)
	{
		std::cout << tb->desc() << " " << tb->volatility() << std::endl;
		count++;
		if (count > 10)
		{
			break;
		}
	}

	return tangles;
}

float Untangle::biasedScore(bool disulphides)
{
	float total = 0;
	float count = 0;
	for (TangledBond &bond : _bonds)
	{
		bool ss = (bond.atom(0)->elementSymbol() == "S" &&
		           bond.atom(1)->elementSymbol() == "S");

		if ((disulphides && ss) || (!disulphides && !ss))
		{
			continue;
		}

		float add = bond.total_score(1.);
		if (add > 1e-6)
		{
			total += add;
			count++;
		}
	}
	
	return total / count;
}

void Untangle::triggerDisplay()
{
	_view->recalculate();
}

void Untangle::save(const std::string &filename)
{
	Write write(_filename, filename, _group);
	write();
//	PdbFile::writeAtoms(_group, filename, true);
}

int Untangle::firstResidue()
{
	int min, max;
	_group->getLimitingResidues(&min, &max);

	return min;
}

Atom *Untangle::atomFor(std::string chain, int resi)
{
	Atom *a = _group->atomByIdName(ResidueId(resi), "CA", chain);
	if (!a)
	{
		return nullptr;
	}

	return a;
}

OpSet<TangledBond *> Untangle::neighbours(TangledBond *first)
{
	OpSet<TangledBond *> collection;
	for (int i = 0; i < 2; i++)
	{
		Atom *atom = first->atom(i);
		
		for (int i = 0; i < atom->bondLengthCount(); i++)
		{
			BondLength *bl = atom->bondLength(i);
			if (!_map[bl]->hasHydrogens())
			{
				collection.insert(_map[bl]);
			}
		}
	}

	return collection;
}
