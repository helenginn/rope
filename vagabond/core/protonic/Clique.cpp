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
	_name = str;
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

void Clique::select(bool left)
{
	if (_beingChosen)
	{
		_beingChosen(left);
	}
}

void Clique::housekeeping(Network &network)
{
	for (const std::string &desc : _descs)
	{
		Probe *probe = network.probeForDesc(desc);
		std::cout << "test: " << probe << std::endl;
		if (probe)
		{
			_probes.insert(probe);
		}
	}

}

void Clique::addProbeDesc(const std::string &str)
{
	_descs.push_back(str);
}
