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
#include "ProbeResult.h"
#include <vagabond/core/Item.h>

#include <nlohmann/json.hpp>
using nlohmann::json;

class Probe;
class Network;
struct ProbeResult;

class Clique : public Item
{
public:
	Clique();
	Clique(const OpSet<Probe *> &probes);
	
	const bool operator<(const Clique &other) const
	{
		if (_probes.size() == 0 && other._probes.size() == 0)
		{
			return _descs < other._descs;
		}
		return _probes < other._probes;
	}
	
	const bool operator==(const Clique &other) const
	{
		if (_probes.size() == 0 && other._probes.size() == 0)
		{
			return _descs == other._descs;
		}
		return _probes == other._probes;
	}
	
	void housekeeping(Network &network);

	std::string name();
	void setName(std::string new_name)
	{
		_name = new_name;
		setDisplayName(name());
	}
	
	const bool hasCustomName() const
	{
		return (_name.length());
	}
	
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

	template <class Container>
	const OpSet<Probe *> probes(const Container &c) const
	{
		OpSet<Probe *> some;
		for (const std::string &desc : c)
		{
			Probe *other = _probes[desc];
			if (other)
			{
				some += other;
			}
		}
		return some;
	}

	void addProbeDesc(const std::string &str);
	OpSet<Probe *> nonWaterProbes();
	
	void removeCommunicationPoints(const OpSet<std::string> &set)
	{
		std::map<std::string, OpSet<std::string>> tmp;
		for (auto it = _communication.begin(); it != _communication.end(); it++)
		{
			it->second -= set;
			if (it->second.size())
			{
				tmp[it->first] = it->second;
			}
		}
		_communication = tmp;

		for (const std::string &desc : set)
		{
			_descToCommune.erase(desc);
		}
	}
	
	void addCommunicationPoints(const std::string &name,
	                            const OpSet<std::string> &set)
	{
		_communication[name] += set;
		for (const std::string &desc : set)
		{
			_descToCommune[desc] = name;
		}
	}
	
	OpSet<std::string> allCommsNames()
	{
		OpSet<std::string> names;
		for (auto it = _communication.begin(); it != _communication.end(); it++)
		{
			names += it->first;
		}
		return names;
	}
	
	std::string groupOfNode(const std::string &desc) const
	{
		if (!_descToCommune.count(desc))
		{
			return "";
		}
		return _descToCommune.at(desc);
	}
	
	std::list<Clique> &subdivisions()
	{
		return _subdivs;
	}

	void setSubdivisions(const OpSet<Clique> &cliques)
	{
		_subdivs = std::list(cliques.begin(), cliques.end());
	}
	
	const std::vector<ProbeResult> &results() const
	{
		return _results;
	}
	
	void setResults(const std::vector<ProbeResult> &results);
private:
	class ProbeKey : public OpSet<Probe *>
	{
	public:
		ProbeKey()
		{

		}

		ProbeKey(const OpSet<Probe *> &probes) : OpSet<Probe *>(probes)
		{

		}

		void insert(Probe *const &probe)
		{
			if (probe)
			{
				OpSet<Probe *>::insert(probe);
				_descs[probe->desc()] = probe;
			}
		}

		Probe *const operator[](const std::string &desc) const
		{
			if (_descs.count(desc))
			{
				return _descs.at(desc);
			}

			return nullptr;
		}
	private:
		std::map<std::string, Probe *> _descs;
	};
	
	ProbeKey _probes{};
	
	std::string _name{};
	std::function<void(bool left)> _beingChosen{};
	std::map<std::string, OpSet<std::string>> _communication;
	std::map<std::string, std::string> _descToCommune;
	std::vector<std::string> _descs;
	std::vector<ProbeResult> _results;
	std::list<Clique> _subdivs;
};

inline void to_json(json &j, const Clique &cl)
{
	j["name"] = cl._name;
	
	std::vector<std::string> descs;
	for (Probe *const &probe : cl.probes())
	{
		descs.push_back(probe->desc());
	}
	
	j["probes"] = descs;
	
	if (cl._communication.size())
	{
		j["communication"] = cl._communication;
	}
	
	if (cl._subdivs.size())
	{
		j["subdivisions"] = cl._subdivs;
	}
}

inline void from_json(const json &j, Clique &cl)
{
	cl._name = j.at("name");
	cl._descs = j.at("probes");
	
	if (j.count("communication"))
	{
		cl._communication = j.at("communication");
		for (auto it = cl._communication.begin();
		     it != cl._communication.end(); it++)
		{
			for (const std::string &desc : it->second)
			{
				cl._descToCommune[desc] = it->first;
			}
		}
	}

	if (j.count("subdivisions"))
	{
		cl._subdivs = j.at("subdivisions");
	}
}

#endif
