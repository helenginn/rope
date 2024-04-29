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

#ifndef __vagabond__Ligand__
#define __vagabond__Ligand__

#include <string>
#include <set>
#include "Instance.h"
#include "ResidueId.h"

class Substance;
class AtomGroup;
class Residue;

class Ligand : public Instance
{
public:
	Ligand();
	Ligand(std::string model_id, AtomGroup *grp);

	virtual std::string desc() const;
	virtual const std::string id() const;

	virtual bool hasSequence() const
	{
		return false;
	}

	virtual bool atomBelongsToInstance(Atom *a);

	virtual Atom *equivalentForAtom(Ligand *other, Atom *atom);

	virtual std::vector<ResidueTorsion> residueTorsionList();

	virtual void grabTorsions(RTAngles &angles,
	                          rope::TorsionType type = rope::RefinedTorsions);

	virtual const Residue *
	localResidueForResidueTorsion(const ResidueTorsion &rt);
protected:
	void collectTorsions();

	friend void to_json(json &j, const Ligand &value);
	friend void from_json(const json &j, Ligand &value);

private:
	std::string _anchorDesc;
	std::string _code;
	std::string _chain;
	std::set<ResidueId> _resids;
	std::vector<ResidueTorsion> _rts;
};

inline void to_json(json &j, const Ligand &value)
{
	j["chain"] = value._chain;
	j["residue_ids"] = value._resids;
	j["anchor_desc"] = value._anchorDesc;
	j["code"] = value._code;
	j["model_id"] = value._model_id;
}

inline void from_json(const json &j, Ligand &value)
{
	value._chain = j.at("chain");
	if (j.count("model_id"))
	{
		value._model_id = j.at("model_id");
	}
	std::set<ResidueId> resids = j.at("residue_ids");
	value._resids = resids;
	value._anchorDesc = j.at("anchor_desc");
	value._code = j.at("code");
}

#endif
