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

#ifndef __vagabond__Network__
#define __vagabond__Network__

#include "Connector.h"
#include "Constraint.h"
#include "Probe.h"

#include <list>
#include <map>

class Probe;

class Network
{
public:
	Network(AtomGroup *group);
	Network();

	template <class Connector>
	auto &add(Connector *const &connector)
	{
		_connectors.push_back(hnet::AnyConnector(connector));
		return *connector;
	}

	template <class Constraint>
	auto &add_constraint(Constraint *const &constraint)
	{
		_constraints.push_back(hnet::AnyConstraint(constraint));
		return *constraint;
	}
	
	const std::list<HydrogenProbe *> &hydrogenProbes() const
	{
		return _hydrogenProbes;
	}
	
	const std::list<BondProbe *> &bondProbes() const
	{
		return _bondProbes;
	}
	
	const std::list<AtomProbe *> &atomProbes() const
	{
		return _atomProbes;
	}
	
	AtomProbe &add_probe(AtomProbe *const &probe);
	BondProbe &add_probe(BondProbe *const &probe);
	HydrogenProbe &add_probe(HydrogenProbe *const &probe);
private:
	void probeAtom(Atom *atom);
	void setupAmineNitrogen(::Atom *atom);
	void setupCarbonylOxygen(::Atom *atom);
	void setupWater(::Atom *atom);

	AtomGroup *findNeighbours(::Atom *centre);
	void attachToNeighbours(::Atom *atom);
	void finaliseContacts(::Atom *atom);
	void mutualExclusions(::Atom *atom);
	void setupAtom(::Atom *atom);

	std::list<hnet::AnyConnector> _connectors;
	std::list<hnet::AnyConstraint> _constraints;
	std::list<AtomProbe *> _atomProbes;
	std::list<HydrogenProbe *> _hydrogenProbes;
	std::list<BondProbe *> _bondProbes;
	
	struct AtomDetails
	{
		hnet::AtomConnector *connector{};

		hnet::CountConnector *strong{};
		hnet::CountConnector *weak{};
		hnet::CountConnector *present{};

		// all bonds regardless of who made them
		std::vector<hnet::BondConnector *> bonds;

		// atoms associated with all bonds
		std::vector<Atom *> bonded_atoms;

		AtomProbe *probe{};
	};

	void prepareSimple(Network::AtomDetails &details, 
	                   const hnet::Count::Values &n_strong,
	                   const hnet::Count::Values &n_weak);

	void prepareCoordinated(Network::AtomDetails &details,
	                        const hnet::Count::Values &n_charge,
	                        const hnet::Count::Values &n_coord_num,
	                        const hnet::Count::Values &n_donations);
	
	std::map<Atom *, AtomDetails> _atomMap;

	AtomGroup *_original = nullptr;
	AtomGroup *_group = nullptr;
};

#endif
