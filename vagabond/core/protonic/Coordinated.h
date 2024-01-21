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

#ifndef __vagabond__Coordinated__
#define __vagabond__Coordinated__

#include <vagabond/utils/OpSet.h>

#include "hnet.h"
#include "Network.h"

class Atom;
class AtomGroup;

namespace hnet
{
typedef std::pair<::Atom *, hnet::BondConnector *> ABPair;

class Coordinated
{
public:
	Coordinated(::Atom *atom, Network &network);
	
	operator ::Atom *const &() const
	{
		return _atom;
	}

	template <class Connector>
	auto &add(Connector *const &connector)
	{
		return _network.add(connector);
	}

	template <class Constraint>
	auto &add_constraint(Constraint *const &constraint)
	{
		return _network.add_constraint(constraint);
	}
	
	hnet::CountConnector &add_zero_or_positive_connector();

	void prepareCoordinated( const Count::Values &n_charge,
	                        const Count::Values &n_coord_num,
	                        const Count::Values &remaining_valency);

	AtomGroup *findNeighbours(AtomGroup *group, const glm::vec3 &v, 
	                          float distance);
	void attachToNeighbours(AtomGroup *searchGroup);
	void mutualExclusions();
	void findBondRanges();
	void attachAdderConstraints();
	void augmentBonding(AtomGroup *search);

	void probeAtom();

	AtomProbe *const &probe() const
	{
		return _probe;
	}
	
	hnet::AtomConnector *const &connector() const
	{
		return _connector;
	}
	
	hnet::CountConnector *const &strong() const
	{
		return _strong;
	}
	
	hnet::CountConnector *const &weak() const
	{
		return _weak;
	}
	
	hnet::CountConnector *const &present() const
	{
		return _present;
	}
	
	hnet::CountConnector *const &absent() const
	{
		return _absent;
	}
	
	hnet::CountConnector *const &expl_bonds() const
	{
		return _expl_bonds;
	}
	
	hnet::CountConnector *const &forced_absent() const
	{
		return _forced_absent;
	}

	std::vector<hnet::BondConnector *> &bonds()
	{
		return _bonds;
	}

	std::vector<::Atom *> &bonded_atoms()
	{
		return _bondedAtoms;
	}
private:
	std::map<::Atom *, Coordinated *> &atomMap() const
	{
		return _network.atomMap();
	}

	OpSet<ABPair> pairSets();

	hnet::AtomConnector *_connector{};

	hnet::CountConnector *_strong{};
	hnet::CountConnector *_weak{};
	hnet::CountConnector *_present{};
	hnet::CountConnector *_absent{};
	hnet::CountConnector *_expl_bonds{};
	hnet::CountConnector *_forced_absent{};

	// all bonds regardless of who made them
	std::vector<hnet::BondConnector *> _bonds;

	// atoms associated with all bonds
	std::vector<::Atom *> _bondedAtoms;
	
	int _coordNum = 0;

	AtomProbe *_probe{};

	Network &_network;
	::Atom *_atom = nullptr;
};

}

#endif
