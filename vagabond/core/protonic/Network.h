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

#include <list>
#include <map>

#include "Connector.h"
#include "Constraint.h"
#include "Probe.h"

class HideComplete;
class Probe;
namespace hnet
{
	class Coordinated;
}

class Network
{
public:
	friend HideComplete;
	Network();
	Network(AtomGroup *group, const std::string &spg_name,
	        const std::array<double, 6> &unit_cell = {});

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
	
	glm::vec3 centre() const;
	
	AtomProbe &add_probe(AtomProbe *const &probe);
	BondProbe &add_probe(BondProbe *const &probe);
	HydrogenProbe &add_probe(HydrogenProbe *const &probe);

	std::map<::Atom *, hnet::Coordinated *> &atomMap()
	{
		return _atomMap;
	}
private:
	void establishAtom(::Atom *atom);

	void setupAmineNitrogen(::Atom *atom);
	void setupCarbonylOxygen(::Atom *atom);
	void setupSingleAlcohol(::Atom *atom);
	void setupLysineAmine(::Atom *atom);
	void setupWater(::Atom *atom);
	void setupAsnGlnNitrogen(::Atom *atom);
	void setupCarboxylOxygen(::Atom *atom);
	void setupHistidine(::Atom *atom);

	void showCarboxylAtom(::Atom *atom);
	void shareCharges(::Atom *left, ::Atom *right,
	                 const hnet::Count::Values &allowable);
	void shareDonors(::Atom *left, ::Atom *right,
	                 const hnet::Count::Values &allowable);

	void findAtomAndNameIt(::Atom *atom, const std::string &atomName, 
	                       const std::string &name);

	void setupAtom(::Atom *atom);

	std::list<hnet::AnyConnector> _connectors;
	std::list<hnet::AnyConstraint> _constraints;
	std::list<AtomProbe *> _atomProbes;
	std::list<HydrogenProbe *> _hydrogenProbes;
	std::list<BondProbe *> _bondProbes;

	std::map<Atom *, hnet::Coordinated *> _atomMap;
	std::map<Atom *, AtomProbe *> _atom2Probe;
	std::map<Atom *, std::vector<HydrogenProbe *> > _h2Probe;

	AtomGroup *_original = nullptr;
	AtomGroup *_symMates = nullptr;
	AtomGroup *_group = nullptr;
	AtomGroup *_groupAndMates = nullptr;
	AtomGroup *_originalAndMates = nullptr;
};

#endif
