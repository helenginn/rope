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
#include <vagabond/c4x/Angular.h>
#include "IndexedSequence.h"

class Atom;
class Entity;
class AtomGroup;
class SequenceComparison;

namespace rope
{
	enum TorsionType
	{
		RefinedTorsions,
		TemporaryTorsions,
	};
}

typedef std::map<Residue *, Residue *> ResidueMap;

class Sequence : public IndexedSequence
{
public:
	Sequence(Atom *anchor);
	Sequence();
	~Sequence();
	Sequence(const Sequence &seq);
	Sequence(std::string str);
	
	Sequence &operator+=(Sequence *other);
	Sequence &operator+=(Residue &res);

	/** @return number of residues */
	size_t size()
	{
		return _residues.size();
	}
	
	/** @return residue associated with the Entity master sequence for the
	 * nth residue.
	 * @param idx index of residue (unrelated to residue number) */
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
	
	/** @return nth residue associated with this sequence 
	 * @param idx index of residue (unrelated to residue number) */
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
	
	std::list<Residue> &residues()
	{
		return _residues;
	}
	
	/** @return residue associated with a ResidueId */
	Residue *residue(ResidueId &id)
	{
		if (_id2Residue.count(id) == 0)
		{
			return nullptr;
		}

		return _id2Residue.at(id);
	}
	
	/** converts entity master residue to local residue
	 * 	@param master pointer to the Entity master residue
	 * 	@return pointer to the corresponding local residue */
	Residue *const local_residue(Residue *master) const;

	/** converts local residue to entity master residue
	 * 	@param local pointer to the corresponding local residue 
	 * 	@return pointer to the Entity master residue */
	Residue *master_residue(Residue *local) const;
	
	/** residue number of the first residue in the sequence */
	int firstNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues.front().as_num();
	}
	
	/** residue number of the last residue in the sequence */
	int lastNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues.back().as_num();
	}
	
	/** residue associated with a ResidueId - does not need to be an
 * 	exact match */
	Residue *residueLike(const ResidueId &other);
	
	/** string of one-letter codes for this sequence */
	std::string str();

	/** called after loading from json to fix references */
	void housekeeping();
	
	/** clear local-to-master residue mapping */
	void clearMaps()
	{
		_map2Local.clear();
		_map2Master.clear();
	}

	/** generate a sequence comparison object with Entity */
	SequenceComparison *newComparison(Entity *entity);

	/** call once to generate a master-to-local mapping from a given
	 * SequenceComparison when establishing relationship to Entity 
	 * master sequence */
	void mapFromMaster(SequenceComparison *sc);
	
	/** if a master-to-local mapping has already been generated, re-generate
	 * the mapping from master-to-local */
	void remapFromMaster(Entity *entity);
	
	/** set entity for sequence without remapping */
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}

	/** name in style t<id>:<desc> to torsion angle (if available) */
	bool torsionByName(const std::string name, Residue **res);

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
	
	void addBufferResidue();

	virtual bool hasResidue(int row, int entry)
	{
		return (entry >= 0 && entry < _residues.size());
	}

	virtual std::string displayString(int row, int entry)
	{
		return residue(entry)->one_letter_code();
	}
	
	/** how many torsion angles have been referenced in the entire sequence? */
	const size_t torsionCount() const;
	
	size_t modelledResidueCount() const;
	
	/** torsion angle names in order of residue / reference, to be called
	 * on the entity master sequence in the case of clustering */
	void addResidueTorsions(std::vector<ResidueTorsion> &headers);

	/** call on entity master sequence to get the corresponding torsion 
	 * angles from a derived/mapped sequence */
	void torsionsFromMapped(Sequence *seq, std::vector<Angular> &vals,
	                        rope::TorsionType type = rope::RefinedTorsions);
	
	AtomGroup *convertToAtoms();
	void addAtomPositionHeaders(std::vector<Atom3DPosition> &headers);
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
    std::list<Residue> residues = j.at("residues");
    value._residues = residues;
    std::list<Residue> master = j.at("master");
    value._master = master;
	
	value.housekeeping();
}

#endif
