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
typedef OpSet<ABPair> PairSet;
typedef std::pair<ABPair, ABPair> CoordSeed;

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
	                          float distance, bool one_sided);
	void attachToNeighbours(AtomGroup *searchGroup);
	void mutualExclusions(AtomGroup *clashCheck);
	void attachAdderConstraints();

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

	size_t bondCount() const
	{
		return _bonds.size();
	}
	
	void addBond(const ABPair &bond);
	
	// to ensure that bonds between asymmetric atoms to symmetry operated atoms
	// are forced to have equal connectivity
	void equilibrateBonds();

	std::vector<hnet::BondConnector *> bonds_only() const;
	
	ABPair bondedSymmetricAtom(::Atom *asymmetric);
	
	const bool &failedCheck() const
	{
		return _failedCheck;
	}

	const PairSet &bonds() const
	{
		return _bonds;
	}

	void eitherOr(const ABPair &first, const ABPair &second);
private:
	OpSet<PairSet> findSeeds();
	ABPair makePossibleHydrogen(const glm::vec3 &pos);
	void comparePairs(OpSet<PairSet> &results,
	                  const ABPair &first, const ABPair &second,
	                  glm::vec3 &centre);
	PairSet developSeed(const PairSet &seed, const PairSet &all,
	                    const PairSet &uninvolved, const glm::vec3 &centre,
	                    AtomGroup *clashCheck, int &fake_atom_count);
	OpSet<PairSet> expandAllSeeds(AtomGroup *clashCheck);

	std::map<::Atom *, Coordinated *> &atomMap() const
	{
		return _network.atomMap();
	}

	hnet::AtomConnector *_connector{};

	hnet::CountConnector *_strong{};
	hnet::CountConnector *_weak{};
	hnet::CountConnector *_present{};
	hnet::CountConnector *_absent{};
	hnet::CountConnector *_expl_bonds{};

	// all bonds regardless of who made them
	PairSet _bonds;
	
	// the atom not involved in hydrogen bonding, but important for coordination
	OpSet<ABPair> _uninvolved{};
	
	int _coordNum = 0;
	bool _failedCheck = false;

	AtomProbe *_probe{};

	Network &_network;
	::Atom *_atom = nullptr;
};

}

#endif
