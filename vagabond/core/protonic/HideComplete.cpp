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

#include <iostream>

#include "Network.h"
#include "HideComplete.h"

HideComplete::HideComplete(Network &network) : _network(network)
{

}

void HideComplete::operator()(bool closed)
{
	if (closed)
	{
		hide_closed();
	}

}

void HideComplete::add_if_unchecked(::Atom *atom, 
                                    std::map<AtomProbe *, bool> &checked,
                      std::list<AtomProbe *> &to_check)
{
	AtomProbe *probe = _network._atom2Probe[atom];

	if (probe && !checked[probe])
	{
		to_check.push_back(probe);
		checked[probe] = true;
	}
}

bool HideComplete::add_connected_hydrogens(std::map<AtomProbe *, 
                                           bool> &checked, ::Atom *atom, 
                                           std::list<AtomProbe *> &to_check,
                                           std::list<Probe *> &to_show)
{
	std::vector<HydrogenProbe *> &hydroprobes = _network._h2Probe[atom];

	for (HydrogenProbe *probe : hydroprobes)
	{
		if (!probe->is_certain())
		{
			return false;
		}
		
		to_show.push_back(probe);
		add_if_unchecked(probe->left().atom(), checked, to_check);
		add_if_unchecked(probe->right().atom(), checked, to_check);
	}
	
	return true;
}

bool add_next_unchecked_probe(std::map<AtomProbe *, bool> &checked,
                              std::list<AtomProbe *> &to_check)
{
	bool found = false;
	for (auto it = checked.begin(); it != checked.end(); it++)
	{
		if (it->second == false)
		{
			to_check.push_back(it->first);
			checked[it->first] = true;
			found = true;
			break;
		}
	}
	
	return found;
}

void make_hidden(std::list<Probe *> to_show)
{
	for (Probe *probe : to_show)
	{
		probe->setAlpha(-1.0f);
	}
}

void HideComplete::hide_closed()
{
	if (_network._atomProbes.size() == 0)
	{
		return;
	}

	std::map<AtomProbe *, bool> checked;
	std::map<AtomProbe *, bool> visible;
	std::list<AtomProbe *> to_check;
	
	for (AtomProbe *probe : _network._atomProbes)
	{
		checked[probe] = false;
		checked[probe] = false;
	}

	std::list<Probe *> to_show;
	add_next_unchecked_probe(checked, to_check);

	bool closed = true;
	while (to_check.size())
	{
		AtomProbe *next = to_check.back();
		to_check.pop_back();
		to_show.push_back(next);
		
		if (!next->is_certain())
		{
			closed = false;
		}
		
		::Atom *atom = next->atom();
		
		if (!add_connected_hydrogens(checked, atom, to_check, to_show))
		{
			closed = false;
		}
		
		if (to_check.size() == 0)
		{
			if (closed)
			{
				make_hidden(to_show);
			}

			to_show.clear();
			closed = true;

			if (!add_next_unchecked_probe(checked, to_check))
			{
				break;
			}
		}
	}

}
