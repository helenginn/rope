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
#include <atomic>
#include <memory>

#include <nlohmann/json.hpp>
using nlohmann::json;

class Probe;
class Network;
class ProbeResult;
class CertainStates;

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
	
	glm::vec3 centroid();

	/** false when centroid() has nothing to average (e.g. an empty
	 * clique) and would just return the {0,0,0} fallback - callers that
	 * recentre a view on centroid() should check this first rather than
	 * refocusing on that arbitrary fallback position. */
	bool hasCentroid() const
	{
		for (Probe *const &pr : _probes)
		{
			if (pr->is_atom() && !pr->is_bulk())
			{
				return true;
			}
		}

		return false;
	}
	
	void housekeeping(Network &network);

	// inverse of housekeeping(): downgrades live Probe* references back to
	// their stable desc() strings and forgets the pointers. Call this on a
	// copy before it outlives the Network that owns those Probes (e.g.
	// before pushing to Model) - housekeeping() only ever adds to _probes
	// from _descs, it never clears stale entries first, so a Clique handed
	// to a future Network must not still be holding pointers into this one.
	void prepareForStorage();

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

	/** the normalised "select using plan" search string that produced
	 * this clique's selection (chain/residue ranges plus a "@N;" radius
	 * prefix, always present even if the user didn't type one), or empty
	 * for a clique that wasn't built from a plan. */
	const std::string &planText() const
	{
		return _planText;
	}

	void setPlanText(const std::string &text)
	{
		_planText = text;
	}
	
	bool is_certain() const;
	
	int num_waters() const;

	friend void to_json(json &j, const Clique &value);
	friend void from_json(const json &j, Clique &value);
	
	void add_probes(const OpSet<Probe *> &probes);
	
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

	void clearCommunicationPoints()
	{
		_communication.clear();
		_descToCommune.clear();
	}

	const OpSet<std::string> &nodeDescsForGroup(const std::string &name)
	{
		return _communication[name];
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
		// SearchAll::run() registers searched subdivisions as Items of
		// this Clique (addItem(&clique), where &clique is the address of
		// the Clique living inside _subdivs). Item::~Item() does not
		// self-deregister, so replacing _subdivs without first removing
		// those entries here would leave this->_items holding dangling
		// pointers into the about-to-be-destroyed old list - only
		// noticed later, whenever something (e.g. browsing cliques)
		// walks this Clique's items. removeItem() is a harmless no-op
		// for any subdivision that was never actually registered.
		for (Clique &old : _subdivs)
		{
			removeItem(&old);
		}

		_subdivs = std::list(cliques.begin(), cliques.end());
	}
	
	void addSubdivision(const Clique &sub)
	{
		_subdivs.push_back(sub);
	}

	/** shared with whichever SearchAll is currently iterating/mutating
	 * this Clique's subdivisions on a worker thread (if any), so any
	 * long-lived view (e.g. HBondAnalysisControl, which may be destroyed
	 * and recreated between opening and returning to this clique) can
	 * check whether it is still unsafe to delete/replace those
	 * subdivisions from the main thread. Not serialized - purely runtime
	 * bookkeeping. */
	std::shared_ptr<std::atomic<bool>> &searchRunning()
	{
		return _searchRunning;
	}

	const std::shared_ptr<CertainStates> &states() const
	{
		return _states;
	}

	void setStates(const std::shared_ptr<CertainStates> &states);
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
	std::string _planText{};
	std::map<std::string, OpSet<std::string>> _communication;
	std::map<std::string, std::string> _descToCommune;
	OpSet<std::string> _descs;
	std::shared_ptr<CertainStates> _states{};
	std::list<Clique> _subdivs;
	std::shared_ptr<std::atomic<bool>> _searchRunning;
};

inline void to_json(json &j, const Clique &cl)
{
	j["name"] = cl._name;

	std::vector<std::string> descs;
	if (cl._probes.size())
	{
		for (Probe *const &probe : cl.probes())
		{
			descs.push_back(probe->desc());
		}
	}
	else
	{
		for (const std::string &desc : cl._descs)
		{
			descs.push_back(desc);
		}
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

	if (cl._planText.size())
	{
		j["plan"] = cl._planText;
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

	if (j.count("plan"))
	{
		cl._planText = j.at("plan");
	}
}

#endif
