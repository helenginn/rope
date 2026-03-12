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

#ifndef __vagabond__Clique__
#define __vagabond__Clique__

#include <vagabond/utils/OpSet.h>
#include "Probe.h"
#include <vagabond/core/Item.h>

#include <nlohmann/json.hpp>
using nlohmann::json;

class Probe;
class Network;

class Clique : public Item
{
public:
	Clique() {};
	Clique(const OpSet<Probe *> &probes);
	
	void housekeeping(Network &network);

	std::string name();
	
	bool is_certain() const;
	
	int num_waters() const;
	
	void setHandleBeingChosen(const std::function<void(bool left)> &chosen)
	{
		_beingChosen = chosen;
	}
	
	virtual void select(bool left);

	friend void to_json(json &j, const Clique &value);
	friend void from_json(const json &j, Clique &value);
	
	const OpSet<Probe *> &probes() const
	{
		return _probes;
	}

	void addProbeDesc(const std::string &str);
private:
	OpSet<Probe *> _probes{};
	
	std::string _name{};
	std::function<void(bool left)> _beingChosen{};
	std::vector<std::string> _descs;
};

inline void to_json(json &j, const Clique &cl)
{
	j["name"] = cl._name;
	
	std::vector<std::string> descs;
	for (Probe *const &probe : cl.probes())
	{
//		if (probe->is_atom())
		{
			descs.push_back(probe->desc());
		}
	}
	
	j["probes"] = descs;
}

inline void from_json(const json &j, Clique &cl)
{
	cl._name = j.at("name");
	cl._descs = j.at("probes");
}

#endif
