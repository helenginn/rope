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

#ifndef __vagabond__Sequence__
#define __vagabond__Sequence__

#include <list>
#include <map>
#include <set>
#include <sstream>
#include "Residue.h"
#include "IndexedSequence.h"

class Atom;
class Entity;
class SequenceComparison;

typedef std::map<Residue *, Residue *> ResidueMap;

class Sequence : public IndexedSequence
{
public:
	Sequence(Atom *anchor);
	Sequence();
	~Sequence();
	Sequence(const Sequence &seq);
	Sequence(std::string str);
	
	Sequence &operator+=(Sequence *&other);
	Sequence &operator+=(Residue &res);

	size_t size()
	{
		return _residues.size();
	}
	
	Residue *master(int idx)
	{
		int i = 0;
		for (Residue &r : _master)
		{
			if (i == idx)
			{
				return &r;
			}
			i++;
		}
		return nullptr;
	}
	
	Residue *residue(int idx)
	{
		int i = 0;
		for (Residue &r : _residues)
		{
			if (i == idx)
			{
				return &r;
			}
			i++;
		}
		return nullptr;
	}
	
	Residue *residue(ResidueId &id)
	{
		if (_id2Residue.count(id) == 0)
		{
			return nullptr;
		}

		return _id2Residue.at(id);
	}
	
	Residue *const local_residue(Residue *const master) const;
	Residue *master_residue(Residue *local) const;
	
	int firstNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues.front().as_num();
	}
	
	int lastNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues.back().as_num();
	}
	
	Residue *residueLike(const ResidueId &other);
	
	std::string str();
	void housekeeping();
	
	void clearMaps()
	{
		_map2Local.clear();
		_map2Master.clear();
	}

	SequenceComparison *newComparison(Entity *entity);
	void mapFromMaster(SequenceComparison *sc);
	void remapFromMaster(Entity *entity);

	friend void to_json(json &j, const Sequence &value);
	friend void from_json(const json &j, Sequence &value);

	virtual size_t rowCount()
	{
		return 1;
	}
	virtual size_t entryCount()
	{
		return size();
	}
	
	virtual Residue *residue(int row, int entry)
	{
		return residue(entry);
	}

	virtual bool hasResidue(int row, int entry)
	{
		return (entry >= 0 && entry < _residues.size());
	}

	virtual std::string displayString(int row, int entry)
	{
		return residue(entry)->one_letter_code();
	}
	
	const size_t torsionCount(bool onlyMain) const;
	
	void addTorsionNames(std::vector<std::string> &names, bool onlyMain);
	void torsionsFromMapped(Sequence *seq, std::vector<float> &vals,
	                        bool onlyMain, float dummy = -1);
	
private:
	void findSequence();
	
	std::list<Residue> _residues;
	std::list<Residue> _master;

	std::map<ResidueId, Residue *> _id2Residue;
	ResidueMap _map2Master;
	ResidueMap _map2Local;
	Atom *_anchor = nullptr;
	Entity *_entity = nullptr;
};

/* sequence */
inline void to_json(json &j, const Sequence &value)
{
	j["residues"] = value._residues;
	j["master"] = value._master;
}

/* sequence */
inline void from_json(const json &j, Sequence &value)
{
	value._residues = j.at("residues");
	value._master = j.at("master");
	
	value.housekeeping();
}

#endif
