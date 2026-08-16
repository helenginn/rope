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
#include <ctime>

#include <nlohmann/json.hpp>
using nlohmann::json;

class Probe;
class Network;
class ProbeResult;
class CertainStates;
class Clique;

/** one call to Subdivide::subdivide()/one() worth of results, kept
 * alongside any earlier/later runs rather than overwriting them (see
 * Clique::subdivisionRuns()) so a user can compare runs made with
 * different settings and choose which one downstream occupancy/
 * signalling-choice/communication analysis should respond to (the
 * `active` flag, exactly one true at a time within a Clique's
 * subdivisionRuns() - see Clique::activeSubdivisionRun()). */
class SubdivisionRun
{
public:
	SubdivisionRun() {}

	std::list<Clique> subdivisions;
	int maxNodes = 0;
	int samplesPerNode = 0;
	bool bruteForce = false;
	bool active = false;
	time_t timestamp = 0;

	/** short auto-generated label for the tickbox row, e.g. "6 nodes,
	 * 3 samples" or "Brute force" - full details (clique count, average
	 * node count, timestamp) belong in a details view instead. */
	std::string description() const
	{
		if (bruteForce)
		{
			return "Brute force";
		}

		return std::to_string(maxNodes) + " nodes, " +
		       std::to_string(samplesPerNode) + " samples";
	}

	friend void to_json(json &j, const SubdivisionRun &value);
	friend void from_json(const json &j, SubdivisionRun &value);
};

// to_json/from_json for SubdivisionRun are defined below Clique's own
// (de)serialization, since they need Clique to be a complete type
// (SubdivisionRun::subdivisions is std::list<Clique>, fine to declare
// with an incomplete type but not to json-(de)serialize with one).

class Clique : public Item
{
public:
	Clique();
	Clique(const OpSet<Probe *> &probes);
	Clique(const Clique &other);
	Clique &operator=(const Clique &other);

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

	/** how many independent Subdivide::subdivide() samples this
	 * subdivision actually represents - always >= 1. Every probe in the
	 * top-level clique starts its own independent random walk, so before
	 * near-duplicate/subset screening this count was implicit (one
	 * subdivision per surviving walk); discarding a walk that collapsed
	 * onto (or into a strict subset of) another one's region loses that
	 * walk's own share of the sampling entirely, silently undercounting
	 * regions where many independent walks happen to collapse together
	 * relative to regions no other walk reached. Incremented once per
	 * walk this subdivision "ate" (an exact-duplicate or subset match -
	 * see Subdivide::subdivide()) instead of discarding it outright, so
	 * that information survives without needing to keep - or pay to
	 * search - the redundant copies themselves. */
	int sampleWeight() const
	{
		return _sampleWeight;
	}

	void setSampleWeight(int weight)
	{
		_sampleWeight = weight;
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

		if (set.count(_watchedDesc))
		{
			_watchedDesc.clear();
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
		_watchedDesc.clear();
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
	
	/** the currently active run's own subdivisions (or an empty list if
	 * there are no runs / none marked active) - kept as the accessor
	 * every existing caller (SearchAll, ViewCorrelations, OccupanciesView,
	 * HBondAnalysisControl) already used before subdivisions could have
	 * more than one run, so occupancy/signalling-choice/communication
	 * analysis transparently follows whichever run is ticked active
	 * without each of those call sites needing to know about runs at
	 * all. */
	std::list<Clique> &subdivisions()
	{
		SubdivisionRun *run = activeSubdivisionRun();
		if (!run)
		{
			static std::list<Clique> empty;
			return empty;
		}

		return run->subdivisions;
	}

	std::list<SubdivisionRun> &subdivisionRuns()
	{
		return _subdivisionRuns;
	}

	/** nullptr if there are no runs, or none is currently marked
	 * active. */
	SubdivisionRun *activeSubdivisionRun();

	/** flips `active` on exactly the given run (which must already be
	 * an element of subdivisionRuns()) and off on every other run. */
	void setActiveSubdivisionRun(SubdivisionRun *run);

	/** appends a fresh run built from a Subdivide::subdivide()/one()
	 * result and makes it the active one, leaving any earlier runs
	 * (and their already-searched states()) untouched and available to
	 * switch back to later. */
	void addSubdivisionRun(const OpSet<Clique> &cliques, int maxNodes,
	                        int samplesPerNode, bool bruteForce);

	/** removes one run (deregistering any Items SearchAll::run()
	 * registered for its subdivisions first, same reasoning as the old
	 * setSubdivisions({}) had) - if the removed run was active, no run
	 * is left active afterwards; the caller/GUI should tick another one
	 * if it wants analysis to keep responding to something. */
	void removeSubdivisionRun(SubdivisionRun *run);

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

	/** shared with whichever SearchAll is currently iterating this
	 * Clique's subdivisions (if any) - stored here rather than only
	 * locally in HBondAnalysisControl's own exhaustive_search job, so
	 * something outside that scene (ProtonNetworkView::cancelAnalysis())
	 * can request cancellation of a search it didn't start and has no
	 * other handle on. Same runtime-only, not-serialized idiom as
	 * searchRunning() above. */
	std::shared_ptr<std::atomic<bool>> &searchCancelled()
	{
		return _searchCancelled;
	}

	const std::shared_ptr<CertainStates> &states() const
	{
		return _states;
	}

	void setStates(const std::shared_ptr<CertainStates> &states);

	/** desc of the single assigned communication-point signal currently
	 * "watched" in CommunicationChoice (empty if none) - serialized (see
	 * to_json()/from_json() below), so it persists across runs the same
	 * way _sampleWeight does; a loaded desc that no longer matches any
	 * current communication point/probe is simply inert (every reader
	 * - CommunicationChoice, ViewCorrelations, OccupanciesView - only
	 * ever compares it against whatever descs currently exist) rather
	 * than needing validation here. */
	const std::string &watchedSignal() const
	{
		return _watchedDesc;
	}

	void setWatchedSignal(const std::string &desc)
	{
		_watchedDesc = desc;
	}

	/** ViewCorrelations::makeSearchButton()'s own residue-range query
	 * text (empty if none) - runtime-only, not serialized: lives here
	 * (rather than as a ViewCorrelations member) purely so it survives
	 * closing and reopening that scene for as long as this Clique stays
	 * in memory, same idiom as watchedSignal()/searchRunning() above.
	 * filterSubdivisions()'s own hidden state already lives on the Clique
	 * tree itself and so already survives on its own - this is only ever
	 * about restoring the visual search-box/cross-button indicator. */
	const std::string &searchText() const
	{
		return _searchText;
	}

	void setSearchText(const std::string &text)
	{
		_searchText = text;
	}
private:
	/** Item::_items/_parent are raw, non-owning pointers that Item's own
	 * (compiler-generated) copy/assignment copies verbatim - fine for
	 * most Items, but _subdivisionRuns holds std::list<Clique> subdivisions
	 * by value, so copying a Clique also deep-copies its subdivisions to
	 * fresh addresses while Item's shallow copy leaves _items/_parent
	 * pointing at the SOURCE's addresses. Left uncorrected, this dangles
	 * as soon as the source is destroyed (e.g. Network::~Network() ->
	 * updateModelCliques() copying into Model, then the live Network's
	 * own _cliques/_subdivisionRuns being destroyed right after) - the
	 * crash this fixes surfaces later, in LineGroup/ItemLine, when
	 * CliqueView's wireDescendants() walks a re-entered
	 * ProtonNetworkView's (by then doubly-copied) clique tree via
	 * items(). Called from both the copy constructor and copy assignment
	 * right after the member-wise copy, to re-home _items/_parent onto
	 * this object's own state. */
	void fixupItemsAfterCopy(const Clique &other);

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
	int _sampleWeight = 1;
	std::map<std::string, OpSet<std::string>> _communication;
	std::map<std::string, std::string> _descToCommune;
	OpSet<std::string> _descs;
	std::shared_ptr<CertainStates> _states{};
	std::list<SubdivisionRun> _subdivisionRuns;
	std::shared_ptr<std::atomic<bool>> _searchRunning;
	std::shared_ptr<std::atomic<bool>> _searchCancelled;
	std::string _watchedDesc;
	std::string _searchText;
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
	
	if (cl._subdivisionRuns.size())
	{
		j["subdivision_runs"] = cl._subdivisionRuns;
	}

	if (cl._planText.size())
	{
		j["plan"] = cl._planText;
	}

	if (cl._sampleWeight != 1)
	{
		j["sample_weight"] = cl._sampleWeight;
	}

	if (cl._watchedDesc.size())
	{
		j["watched_signal"] = cl._watchedDesc;
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

	if (j.count("subdivision_runs"))
	{
		cl._subdivisionRuns = j.at("subdivision_runs").get<std::list<SubdivisionRun>>();
	}
	else if (j.count("subdivisions"))
	{
		// pre-multi-run save format: a flat list<Clique>, migrated into
		// a single active run so old saved models keep working.
		SubdivisionRun run;
		run.subdivisions = j.at("subdivisions").get<std::list<Clique>>();
		run.active = true;
		cl._subdivisionRuns.push_back(run);
	}

	if (j.count("plan"))
	{
		cl._planText = j.at("plan");
	}

	if (j.count("sample_weight"))
	{
		cl._sampleWeight = j.at("sample_weight");
	}

	if (j.count("watched_signal"))
	{
		cl._watchedDesc = j.at("watched_signal");
	}
}

inline void to_json(json &j, const SubdivisionRun &r)
{
	j["subdivisions"] = r.subdivisions;
	j["max_nodes"] = r.maxNodes;
	j["samples_per_node"] = r.samplesPerNode;
	j["brute_force"] = r.bruteForce;
	j["active"] = r.active;
	j["timestamp"] = (long long)r.timestamp;
}

inline void from_json(const json &j, SubdivisionRun &r)
{
	r.subdivisions = j.at("subdivisions").get<std::list<Clique>>();
	r.maxNodes = j.value("max_nodes", 0);
	r.samplesPerNode = j.value("samples_per_node", 0);
	r.bruteForce = j.value("brute_force", false);
	r.active = j.value("active", false);
	r.timestamp = (time_t)j.value("timestamp", (long long)0);
}

#endif
