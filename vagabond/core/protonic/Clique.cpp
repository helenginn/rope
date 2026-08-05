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
_planText(other._planText), _communication(other._communication),
_descToCommune(other._descToCommune), _descs(other._descs),
_states(other._states), _subdivs(other._subdivs),
_searchRunning(other._searchRunning), _watchedDesc(other._watchedDesc)
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
	_communication = other._communication;
	_descToCommune = other._descToCommune;
	_descs = other._descs;
	_states = other._states;
	_subdivs = other._subdivs;
	_searchRunning = other._searchRunning;
	_watchedDesc = other._watchedDesc;

	fixupItemsAfterCopy(other);
	return *this;
}

void Clique::fixupItemsAfterCopy(const Clique &other)
{
	// whatever Item's own copy just gave _items/_parent, it points at
	// OTHER's state, not this object's freshly-copied _subdivs above -
	// drop it all before re-registering the right addresses.
	clearParent();

	std::vector<Item *> stale = items();
	for (Item *item : stale)
	{
		removeItem(item);
	}

	// _subdivs was copied element-for-element from other._subdivs, so
	// the two lists correspond 1:1 in the same order - re-register only
	// the subdivisions that OTHER itself had registered as items
	// (SearchAll::run()'s addItem(&clique) once a subdivision has
	// actually been searched), pointing at our own copies instead.
	auto srcIt = other._subdivs.begin();
	auto dstIt = _subdivs.begin();
	for (; srcIt != other._subdivs.end(); srcIt++, dstIt++)
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

	std::list<Clique> cleaned;
	for (Clique cl : _subdivs)
	{
		cl.housekeeping(network);
		cleaned.push_back(cl);
	}
	_subdivs = cleaned;
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

	for (Clique &sub : _subdivs)
	{
		sub.prepareForStorage();
	}
}

void Clique::addProbeDesc(const std::string &str)
{
	_descs += str;
}

OpSet<Probe *> Clique::nonWaterProbes()
{
	OpSet<Probe *> nonwater;
	for (Probe *pr : _probes)
	{
		if (pr->is_atom())
		{
			bool water = (pr->atom()->code() == "HOH");
			if (!pr->is_certain())
			{
				nonwater.insert(pr);
			}
			if (pr->is_certain())
			{
				for (Probe *connected : pr->others())
				{
					if (!connected->is_covalent() && !connected->is_certain())
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

