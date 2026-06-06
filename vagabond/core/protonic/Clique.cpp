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
		if (pr->is_atom())
		{
			Atom *atom = pr->atom();
			sum += atom->initialPosition();
			count++;
		}
	}
	
	return sum / count;
}

void Clique::setStates(CertainStates *const &states)
{
	_states = states;
}

void Clique::add_clique(const Clique &clique)
{
	add_probes(clique.probes());
	
	for (auto &pair : clique._communication)
	{
		_communication[pair.first] += pair.second;
	}

	for (const std::string &desc : clique._descs)
	{
		addProbeDesc(desc);
	}

	for (auto &pair : clique._descToCommune)
	{
		_descToCommune[pair.first] = pair.second;
	}
	

}
