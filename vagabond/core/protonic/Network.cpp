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
#include "AtomGroup.h"

using namespace hnet;

auto find_close(::Atom *const &ref, float threshold)
{
	return [ref, threshold](::Atom *const &atom)
	{
		glm::vec3 pos = ref->initialPosition();
		glm::vec3 init = atom->initialPosition();

		for (int i = 0; i < 3; i++)
		{
			if (fabs(init[i] - pos[i]) > threshold)
			{
				return false;
			}
		}
		
		if (atom->isConnectedToAtom(ref))
		{
			return false;
		}
		
		if (atom->bondsBetween(ref, 5) >= 0)
		{
			return false;
		}
		
		return (glm::length(init - pos) < threshold);
	};
}

AtomGroup *Network::findNeighbours(::Atom *centre)
{
	auto filter_in = find_close(centre, 3.1);

	return _group->new_subset(filter_in);
}

Network::Network(AtomGroup *group)
{
	_group = group->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() == "N" || atom->elementSymbol() == "O");
	});

	for (::Atom *const &atom : _group->atomVector())
	{
		if (atom->atomName() == "N")
		{
			std::cout << atom->desc() << std::endl;
			AtomGroup *search = findNeighbours(atom);
			
			for (::Atom *const &neighbour : search->atomVector())
			{
				std::cout << "\t" << neighbour->desc() << std::endl;
			}
		}
	}
}

Network::Network()
{
	/* oxygen */
	AtomConnector &o = add(new AtomConnector(hnet::Atom::Oxygen));
	AtomProbe &oProbe = add_probe(new AtomProbe(o));

	/* nitrogen */
	AtomConnector &n = add(new AtomConnector(hnet::Atom::Nitrogen));
	AtomProbe &nProbe = add_probe(new AtomProbe(n));
	nProbe.setPosition({2.9, 0.0});
	
//	IntegerConnector &charge = add(new IntegerConnector(0));

	/* hydrogen */
	BondConnector &h_n = add(new BondConnector());
	BondConnector &o_h = add(new BondConnector(Bond::Strong));
	HydrogenConnector &h = add(new HydrogenConnector());

	HydrogenProbe &hProbe = add_probe(new HydrogenProbe(h, oProbe, nProbe));

	add_probe(new BondProbe(o_h, oProbe, hProbe));
	add_probe(new BondProbe(h_n, hProbe, nProbe));

	AtomConnector &o2 = add(new AtomConnector(hnet::Atom::Oxygen));
	AtomProbe &o2Probe = add_probe(new AtomProbe(o2));
	o2Probe.setPosition({5.0, -1.0});

	HydrogenConnector &h2 = add(new HydrogenConnector(Hydrogen::Present));
	HydrogenProbe &h2Probe = add_probe(new HydrogenProbe(h2, o2Probe, nProbe));

	BondConnector &n_h = add(new BondConnector());
	BondConnector &h_o = add(new BondConnector());

	add_probe(new BondProbe(n_h, nProbe, h2Probe));
	add_probe(new BondProbe(h_o, h2Probe, o2Probe));

	add_constraint(new AtomToBond(o, o_h));
	add_constraint(new HydrogenBond(o_h, h, h_n));
	add_constraint(new AtomToBond(n, h_n));
	add_constraint(new HydrogenBond(n_h, h2, h_o));
	add_constraint(new AtomToBond(n, n_h));
	add_constraint(new AtomToBond(o2, h_o));
	
	std::cout << "First hydrogen: " << h.value() << std::endl;
	std::cout << "First probe: " << hProbe.display() << std::endl;
}

HydrogenProbe &Network::add_probe(HydrogenProbe *const &probe)
{
	_hydrogenProbes.push_back(probe);
	return *probe;
}

AtomProbe &Network::add_probe(AtomProbe *const &probe)
{
	_atomProbes.push_back(probe);
	return *probe;
}

BondProbe &Network::add_probe(BondProbe *const &probe)
{
	_bondProbes.push_back(probe);
	return *probe;
}
