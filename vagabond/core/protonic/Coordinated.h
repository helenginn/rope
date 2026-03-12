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

#include "Network.h"
#include "alignment.h"

class Atom;
class AtomGroup;

namespace hnet
{
class Coordinated
{
public:
	Coordinated(Network &network, ::Atom *atom, char conf = '\0');
	
	operator ::Atom *const &() const
	{
		return _atomConf.ptr;
	}
	
	const AtomConf &atomConf() const
	{
		return _atomConf;
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

	static OpSet<AtomConf> expandGroupToSet(AtomGroup *group);
	OpSet<AtomConf> findNeighbours(const OpSet<AtomConf> &group,
	                               const glm::vec3 &v, 
	                               float distance, bool one_sided);

	void attachToNeighbours(AtomGroup *searchGroup);
	void mutualExclusions(AtomGroup *clashCheck);
	void attachAdderConstraints();
	void clashLogic(OpSet<AtomConf> &clash_check);

	void probeAtom();
	
	glm::vec3 atomic_position();

	AtomProbe *const &probe() const
	{
		return _probe;
	}
	
	hnet::ExistenceConnector *const &existence() const
	{
		return _existence;
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
	void findSymmetricallyRelatedBonds();

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
	
	hnet::CountConnector *const &donors() const
	{
		return _donors;
	}
	
	hnet::CountConnector *const &charge() const
	{
		return _charge;
	}

	void eitherOr(const ABPair &first, const ABPair &second);

	const std::map<BondConnector *, ExistenceConnector *> &bond2Exist() const
	{
		return _bond2Exist;
	}
private:
	OpSet<PairSet> findSeeds(int coord_num);
	::Atom *makeHydrogen(const glm::vec3 &pos);
	ABPair makePossibleHydrogen(const glm::vec3 &pos);
	bool acceptableHydrogenAngle(const glm::vec3 &hydrogen, int coordNum);
	void comparePairs(OpSet<PairSet> &results,
	                  const ABPair &first, const ABPair &second,
	                  glm::vec3 &centre, int coordNum);
	AcceptableGroup developSeed(const PairSet &seed, const PairSet &all,
	                            const glm::vec3 &centre,
	                            OpSet<AtomConf> &clashCheck,
	                            int &fake_atom_count, int coord_num);
	OpSet<AcceptableGroup> expandAllSeeds(OpSet<AtomConf> &clashCheck,
	                                      const PairSet &uninvolved_group,
	                                      PairSet &all_used, int coord_num);
	void applyRestrictionsToUnbrokenBonds
	(const std::map<int, std::vector<int>> &coord_state_broken_bond_counts);
	
	OpSet<ABPair> uninvolvedCoordinators();

	std::map<hnet::AtomConf, Coordinated *> &atomMap() const
	{
		return _network.atomMap();
	}

	hnet::AtomConnector *_connector{};
	hnet::ExistenceConnector *_existence{};

	hnet::CountConnector *_coord_num{};
	hnet::CountConnector *_charge{};
	hnet::CountConnector *_donors{};

	hnet::CountConnector *_strong{};
	hnet::CountConnector *_weak{};
	hnet::CountConnector *_present{};
	hnet::CountConnector *_absent{};
	hnet::CountConnector *_expl_bonds{};

	void clashLogic();

	// all bonds regardless of who made them
	PairSet _bonds;
	
	// the atom not involved in hydrogen bonding, but important for coordination
	OpSet<PairSet> _uninvolved_groups{};
	OpSet<ABPair> _uninvolved{};
	
	int _coordNum = 0;
	bool _failedCheck = false;

	AtomProbe *_probe{};
	
	::Atom *const &atom() const
	{
		return _atomConf.ptr;
	}

	std::map<BondConnector *, ExistenceConnector *> _bond2Exist;
	Network &_network;
	AtomConf _atomConf = {nullptr, '\0'};
	OpSet<AtomConf> _neighbours{};
};

}

#endif
