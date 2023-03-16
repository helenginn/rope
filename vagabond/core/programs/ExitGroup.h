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

class Atom;
#include <json/json.hpp>
using nlohmann::json;

namespace rope
{
struct ExitGroup
{
public:
	struct Flaggable
	{
		std::string name;
		int idx;
		bool central;
		bool entry;
		Atom *ptr;
	};

	Flaggable *entry()
	{
		for (Flaggable &f : atoms)
		{
			if (f.entry)
			{
				return &f;
			}
		}

		return nullptr;

	}

	Flaggable *central()
	{
		for (Flaggable &f : atoms)
		{
			if (f.central)
			{
				return &f;
			}
		}

		return nullptr;
	}

	bool hasAtom(Atom *ptr)
	{
		for (Flaggable &f : atoms)
		{
			if (f.ptr == ptr)
			{
				return true;
			}
		}

		return false;
	}

	void addCentral(std::string name)
	{
		atoms.push_back(Flaggable{name, -1, true, false, nullptr});
	}

	void add(std::string name, bool entry = false)
	{
		atoms.push_back(Flaggable{name, -1, false, entry, nullptr});
	}

	bool allFlagged()
	{
		for (Flaggable &f : atoms)
		{
			if (f.idx < 0)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<Flaggable> atoms;
};

inline void to_json(json &j, const ExitGroup::Flaggable &value)
{
	j["name"] = value.name;
	j["central"] = value.central;
	j["entry"] = value.entry;
}

inline void from_json(const json &j, ExitGroup::Flaggable &value)
{
	value.name = j.at("name");
	value.central = j.at("central");
	value.entry = j.at("entry");
	value.idx = -1;
	value.ptr = nullptr;
}

inline void to_json(json &j, const ExitGroup &value)
{
	j["atoms"] = value.atoms;
}

inline void from_json(const json &j, ExitGroup &value)
{
	std::vector<ExitGroup::Flaggable> atoms = j.at("atoms");
	value.atoms = atoms;
}

};

