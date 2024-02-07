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

#ifndef __vagabond__HideComplete__
#define __vagabond__HideComplete__

#include <map>
#include <list>

class Network;
class AtomProbe;
class Atom;
class Probe;

class HideComplete
{
public:
	HideComplete(Network &network);

	void operator()(bool closed = true);
private:
	bool add_connected_hydrogens(std::map<AtomProbe *, 
	                             bool> &checked, ::Atom *atom, 
	                             std::list<AtomProbe *> &to_check,
	                             std::list<Probe *> &to_show);

	void add_if_unchecked(::Atom *atom, 
	                      std::map<AtomProbe *, bool> &checked,
	                      std::list<AtomProbe *> &to_check);
	void hide_closed();

	Network &_network;

};

#endif
