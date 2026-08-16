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

#include "Network.h"
#include "Clique.h"
#include "Probe.h"

Clique::Clique(const OpSet<Probe *> &probes) : _probes(probes)
{
	setSelectable(true);
}

Clique::Clique()
{
	setSelectable(true);
}

Clique::Clique(const Clique &other)
: Item(other), _probes(other._probes), _name(other._name),
_planText(other._planText), _sampleWeight(other._sampleWeight),
_communication(other._communication),
_descToCommune(other._descToCommune), _descs(other._descs),
_states(other._states), _subdivisionRuns(other._subdivisionRuns),
_searchRunning(other._searchRunning), _watchedDesc(other._watchedDesc),
_searchText(other._searchText)
{
	fixupItemsAfterCopy(other);
}

Clique &Clique::operator=(const Clique &other)
{
	if (this == &other)
	{
		return *this;
	}

	Item::operator=(other);
	_probes = other._probes;
	_name = other._name;
	_planText = other._planText;
	_sampleWeight = other._sampleWeight;
	_communication = other._communication;
	_descToCommune = other._descToCommune;
	_descs = other._descs;
	_states = other._states;
	_subdivisionRuns = other._subdivisionRuns;
	_searchRunning = other._searchRunning;
	_watchedDesc = other._watchedDesc;
	_searchText = other._searchText;

	fixupItemsAfterCopy(other);
	return *this;
}

void Clique::fixupItemsAfterCopy(const Clique &other)
{
	// whatever Item's own copy just gave _items/_parent, it points at
	// OTHER's state, not this object's freshly-copied _subdivisionRuns
	// above - drop it all before re-registering the right addresses.
	clearParent();

	std::vector<Item *> stale = items();
	for (Item *item : stale)
	{
		removeItem(item);
	}

	// _subdivisionRuns, and each run's own subdivisions, were copied
	// element-for-element from other's, so the corresponding lists line
	// up 1:1 in the same order - re-register only the subdivisions that
	// OTHER itself had registered as items (SearchAll::run()'s
	// addItem(&clique) once a subdivision has actually been searched),
	// pointing at our own copies instead.
	auto srcRunIt = other._subdivisionRuns.begin();
	auto dstRunIt = _subdivisionRuns.begin();
	for (; srcRunIt != other._subdivisionRuns.end(); srcRunIt++, dstRunIt++)
	{
		auto srcIt = srcRunIt->subdivisions.begin();
		auto dstIt = dstRunIt->subdivisions.begin();
		for (; srcIt != srcRunIt->subdivisions.end(); srcIt++, dstIt++)
		{
			bool wasItem = false;
			for (Item *item : other.items())
			{
				if (item == &(*srcIt))
				{
					wasItem = true;
					break;
				}
			}

			if (wasItem)
			{
				addItem(&(*dstIt));
			}
		}
	}
}

void Clique::add_probes(const OpSet<Probe *> &probes)
{
	_probes += probes;
}

int Clique::num_waters() const
{
	int waters = 0;
	for (Probe *const &probe : _probes)
	{
		if (probe->atom())
		{
			bool water = (probe->atom()->code() == "HOH");
			if (water)
			{
				waters++;
			}
		}
	}

	return waters;
}

std::string Clique::name()
{
	if (_name.length())
	{
		return _name;
	}

	std::string str;

	int waters = 0;
	int protein = 0;
	for (Probe *const &probe : _probes)
	{
		if (!probe->is_atom())
		{
			continue;
		}
		else if (probe->atom())
		{
			bool water = (probe->atom()->code() == "HOH");
			(water ? waters : protein)++;
		}
	}
	
	str += std::to_string(protein) + " protein atoms, ";
	str += std::to_string(waters) + " waters";
	return str;
}

bool Clique::is_certain() const
{
	for (Probe *const &probe : _probes)
	{
		if (!probe->is_certain())
		{
			return false;
		}
	}
	return true;
}

void Clique::housekeeping(Network &network)
{
	for (const std::string &desc : _descs)
	{
		Probe *probe = network.probeForDesc(desc);
		if (probe)
		{
			_probes.insert(probe);
		}
		else
		{
			std::cout << "Warning! Missing probe for desc: " << desc << std::endl;
			network.promptReclique();
		}
	}
	_descs = {};

	for (SubdivisionRun &run : _subdivisionRuns)
	{
		std::list<Clique> cleaned;
		for (Clique cl : run.subdivisions)
		{
			cl.housekeeping(network);
			cleaned.push_back(cl);
		}
		run.subdivisions = cleaned;
	}
}

void Clique::prepareForStorage()
{
	for (Probe *const &probe : _probes)
	{
		_descs += probe->desc();
	}
	_probes = ProbeKey();

	// this Clique is a copy that may outlive whatever registered as a
	// responder on it (e.g. CliqueView, via insertClique) - the copy
	// carries the same pointer forward without the original ever being
	// told, so when that object is later destroyed nothing removes it
	// here, leaving a dangling pointer for the next session's
	// triggerResponse() to call. clearResponders() only empties the list,
	// it never dereferences the pointers, so this is safe regardless of
	// whether they are still valid.
	clearResponders();

	for (SubdivisionRun &run : _subdivisionRuns)
	{
		for (Clique &sub : run.subdivisions)
		{
			sub.prepareForStorage();
		}
	}
}

SubdivisionRun *Clique::activeSubdivisionRun()
{
	for (SubdivisionRun &run : _subdivisionRuns)
	{
		if (run.active)
		{
			return &run;
		}
	}

	return nullptr;
}

void Clique::setActiveSubdivisionRun(SubdivisionRun *run)
{
	for (SubdivisionRun &candidate : _subdivisionRuns)
	{
		candidate.active = (&candidate == run);
	}
}

void Clique::addSubdivisionRun(const OpSet<Clique> &cliques, int maxNodes,
                                int samplesPerNode, bool bruteForce)
{
	for (SubdivisionRun &run : _subdivisionRuns)
	{
		run.active = false;
	}

	SubdivisionRun run;
	run.subdivisions = std::list<Clique>(cliques.begin(), cliques.end());
	run.maxNodes = maxNodes;
	run.samplesPerNode = samplesPerNode;
	run.bruteForce = bruteForce;
	run.active = true;
	run.timestamp = time(nullptr);

	_subdivisionRuns.push_back(run);
}

void Clique::removeSubdivisionRun(SubdivisionRun *run)
{
	for (auto it = _subdivisionRuns.begin(); it != _subdivisionRuns.end(); it++)
	{
		if (&(*it) != run)
		{
			continue;
		}

		// same reasoning as the old setSubdivisions({}) had: Items may
		// have been registered (SearchAll::run()'s addItem(&clique))
		// against addresses inside this run's subdivisions list, which
		// is about to be destroyed.
		for (Clique &sub : it->subdivisions)
		{
			removeItem(&sub);
		}

		_subdivisionRuns.erase(it);
		return;
	}
}

void Clique::addProbeDesc(const std::string &str)
{
	_descs += str;
}

namespace
{
	// symmetry mates now resolve their own H-bonding independently rather
	// than being forced equal to their mother atom (see Network::Network()),
	// but they still shouldn't be offered as choosable Communication Choice
	// signals - a mate's identity (which residue/copy of the crystal it
	// belongs to) isn't something a user picking a "signal" name is meant to
	// have to reason about.
	bool is_symmetry_mate(Probe *const &probe)
	{
		if (probe->atom())
		{
			return probe->atom()->symmetryCopyOf() != nullptr;
		}
		if (probe->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(probe);
			Atom *l = bp->left().atom();
			Atom *r = bp->right().atom();
			return (l && l->symmetryCopyOf()) || (r && r->symmetryCopyOf());
		}
		return false;
	}
}

OpSet<Probe *> Clique::nonWaterProbes()
{
	OpSet<Probe *> nonwater;
	for (Probe *pr : _probes)
	{
		if (pr->is_atom())
		{
			if (is_symmetry_mate(pr))
			{
				continue;
			}

			bool water = (pr->atom()->code() == "HOH");
			if (!pr->is_certain())
			{
				nonwater.insert(pr);
			}
			if (pr->is_certain())
			{
				for (Probe *connected : pr->others())
				{
					// charge probes (Network::shareCharges()/Coordinated::
					// add_charge_display()) are wired into others() so they
					// join subnetworks/ExhaustiveSearch, but aren't
					// choosable Communication Choice signals yet - kept
					// out here deliberately, not because of a bug (see
					// CertainStates::correlate()'s get_index(), which now
					// handles ChargeType).
					if (!connected->is_covalent() && !connected->is_certain() &&
					    !connected->is_charge() &&
					    !is_symmetry_mate(connected) &&
					    !connected->is_placeholder())
					{
						nonwater.insert(connected);
					}
				}
			}
		}
	}

	return nonwater;
}

glm::vec3 Clique::centroid()
{
	glm::vec3 sum{};
	float count = 0;
	for (Probe *pr : _probes)
	{
		if (pr->is_atom() && !pr->is_bulk())
		{
			Atom *atom = pr->atom();
			sum += atom->initialPosition();
			count++;
		}
	}

	if (count == 0)
	{
		// no atom probes to average (e.g. an empty clique) - sum / 0
		// would be NaN, and every caller feeds this straight into a
		// camera shift, so a NaN here silently corrupts the view
		// transform rather than failing visibly.
		return glm::vec3{};
	}

	return sum / count;
}

void Clique::setStates(const std::shared_ptr<CertainStates> &states)
{
	_states = states;
}

