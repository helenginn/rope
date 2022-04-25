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
#include <vagabond/utils/FileReader.h>
#include "ResidueId.h"

#include <json/json.hpp>
using nlohmann::json;

class Sequence;

class Residue
{
public:
	Residue(ResidueId num, std::string code, std::string chain);
	Residue() {};

	void setSequence(Sequence *seq)
	{
		_sequence = seq;
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
	
	const std::string desc() const
	{
		return _code + i_to_str(as_num());

	}
	
	friend void to_json(json &j, const Residue &value);
	friend void from_json(const json &j, Residue &value);
private:
	ResidueId _id;
	std::string _code;
	std::string _chain;
	
	Sequence *_sequence = nullptr;
};

inline void to_json(json &j, const ResidueId &id)
{
	j = json{{"num",  id.num}, {"insert", id.insert}};
}

inline void from_json(const json &j, ResidueId &id)
{
	j.at("num").get_to(id.num);
	j.at("insert").get_to(id.insert);
}

/* residue */
inline void to_json(json &j, const Residue &value)
{
	j["id"] = value._id;
	j["chain"] = value._chain;
	j["code"] = value._code;
}

/* residue */
inline void from_json(const json &j, Residue &value)
{
	j.at("id").get_to(value._id);
	j.at("chain").get_to(value._chain);
	j.at("code").get_to(value._code);
}

#endif
