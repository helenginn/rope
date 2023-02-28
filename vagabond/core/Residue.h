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

#ifndef __vagabond__Residue__
#define __vagabond__Residue__

#include <string>
#include <set>
#include <vagabond/utils/FileReader.h>
#include "TorsionRef.h"

#include <json/json.hpp>
using nlohmann::json;

class Sequence;

class Residue
{
public:
	Residue(ResidueId num, std::string code, std::string chain);
	Residue() {};
	
	void setNothing(bool nothing)
	{
		_nothing = nothing;
	}
	
	const bool &nothing() const
	{
		return _nothing;
	}

	void setSequence(Sequence *seq)
	{
		_sequence = seq;
	}
	
	const Sequence *sequence() const
	{
		return _sequence;
	}
	
	const size_t torsionCount() const;
	
	const std::set<TorsionRef> &torsions() const
	{
		return _refs;
	}

	int as_num() const
	{
		return _id.as_num();
	}

	/** return sequence number and/or insertion character
	 * @return formatted string of number followed by insertion e.g. "65A" */
	const ResidueId &id() const
	{
		return _id;
	}
	
	const std::string one_letter_code() const;

	/** @return three letter code */
	const std::string &code() const
	{
		return _code;
	}

	/** @return chain identifier */
	const std::string &chain() const
	{
		return _chain;
	}
	
	const std::set<std::string> &atomNames() const
	{
		return _atomNames;
	}
	
	bool supplyRefinedAngle(std::string desc, double angle, bool tmp = false);
	
	const std::string desc() const;
	
	void housekeeping();
	
	bool hasTorsionRef(TorsionRef &ref);
	
	/* note that this will only update the current residue, and not affect
	 * any associated master residues. */
	void addTorsionRef(TorsionRef &ref);
	TorsionRef copyTorsionRef(const std::string &desc);
	void replaceTorsionRef(TorsionRef &newRef);
	
	friend void to_json(json &j, const Residue &value);
	friend void from_json(const json &j, Residue &value);
private:
	ResidueId _id{};
	std::string _code;
	std::string _chain;
	
	bool _nothing = false;
	Sequence *_sequence = nullptr;
	std::set<TorsionRef> _refs;
	std::set<std::string> _atomNames;
};

/* residue */
inline void to_json(json &j, const Residue &value)
{
	j["id"] = value._id;
	j["chain"] = value._chain;
	j["code"] = value._code;
	j["nout"] = value._nothing; // saves a lot of characters
	j["torsions"] = value._refs;
}

/* residue */
inline void from_json(const json &j, Residue &value)
{
	j.at("id").get_to(value._id);
	j.at("chain").get_to(value._chain);
	j.at("code").get_to(value._code);
	
	try
	{
		if (j.count("nothing"))
		{
			j.at("nothing").get_to(value._nothing);
		}
		else if (j.count("nout"))
		{
			j.at("nout").get_to(value._nothing);
		}
	}
	catch (...)
	{

	}
	
	try
	{
		j.at("torsions").get_to(value._refs);
	}
	catch (...)
	{

	}
	
	value.housekeeping();
}

class Entity;

struct Atom3DPosition
{
	std::string atomName;
	Residue *residue = nullptr;
	Entity *entity = nullptr;
	
	std::string desc() const
	{
		if (residue == nullptr)
		{
			std::string id = "p-null:" + atomName;
			return id;
		}

		std::string id = "p" + residue->id().as_string() + ":" + atomName;
		return id;
	}
	
};

#endif
