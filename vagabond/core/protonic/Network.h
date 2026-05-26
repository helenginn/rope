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

#include <vagabond/utils/UndoStack.h>
#include "Connector.h"
#include "Constraint.h"
#include "Clique.h"
#include "Probe.h"

class HideComplete;
class Probe;
class Model;

namespace hnet
{
	class Coordinated;
}

class Network
{
public:
	friend HideComplete;
	Network();
	~Network();
	Network(AtomGroup *group, const std::string &spg_name,
	        const std::array<double, 6> &unit_cell = {}, 
	        Model *const &model = {});

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
	
	UndoStack &undoStack()
	{
		return _undoStack;
	}
	
	glm::vec3 centre() const;
	
	AtomProbe &add_probe(AtomProbe *const &probe);
	BondProbe &add_probe(BondProbe *const &probe);
	CountProbe &add_probe(CountProbe *const &probe);
	HydrogenProbe &add_probe(HydrogenProbe *const &probe);
	
	void addNewHydrogen(hnet::AtomConf hydrogen, hnet::Coordinated *coord);
	
	AtomGroup *atoms()
	{
		return _original;
	}

	AtomGroup *extraHydrogens()
	{
		return _extraHydrogens;
	}

	std::map<hnet::AtomConf, hnet::Coordinated *> &atomMap()
	{
		return _atomMap;
	}
	
	Probe *probeForDesc(const std::string &desc);
	
	AtomProbe *probeForAtom(const hnet::AtomConf &ac)
	{
		if (_atom2Probe.count(ac))
		{
			return _atom2Probe.at(ac);
		}
		return nullptr;
	}
	
	Clique *newClique(const OpSet<Probe *> &probes);
	
	std::list<Clique> &cliques()
	{
		return _cliques;
	}
	
	void updateModelCliques();
	void firstOrderLogic();
	
	void promptReclique();
	bool _reclique = false;

	AtomGroup *assignCertainHydrogens(std::ostringstream &ss);
private:
	void establishAtom(::Atom *atom);

	void setupInactiveAtom(hnet::AtomConf atom);
	bool setupAmineNitrogen(hnet::AtomConf atom);
	bool setupCarbonylOxygen(hnet::AtomConf atom);
	bool setupSingleAlcohol(hnet::AtomConf atom);
	bool setupLysineAmine(hnet::AtomConf atom);
	bool setupWater(hnet::AtomConf atom);
	bool setupArginine(hnet::AtomConf atom);
	bool setupAsnGlnNitrogen(hnet::AtomConf atom);
	bool setupCarboxylOxygen(hnet::AtomConf atom);
	bool setupHistidine(hnet::AtomConf atom);
	bool setupTryptophan(hnet::AtomConf atom);
	bool setupMethionine(hnet::AtomConf atom);


	void shareCharges(hnet::AtomConf left, hnet::AtomConf right,
	                 const hnet::Count::Values &allowable);
	void shareStrong(hnet::AtomConf left, hnet::AtomConf right,
	                 const hnet::Count::Values &allowable);

	void setupAtom(hnet::AtomConf atom);

	std::list<hnet::AnyConnector> _connectors;
	std::list<hnet::AnyConstraint> _constraints;
	std::list<AtomProbe *> _atomProbes;
	std::list<HydrogenProbe *> _hydrogenProbes;
	std::list<BondProbe *> _bondProbes;
	std::list<CountProbe *> _countProbes;
	std::map<std::string, Probe *> _desc2Probe;
	
	std::list<Clique> _cliques;
	Model *_model{};

	std::map<hnet::AtomConf, hnet::Coordinated *> _atomMap;
	std::map<hnet::AtomConf, AtomProbe *> _atom2Probe;
	std::map<hnet::AtomConf, std::vector<HydrogenProbe *> > _h2Probe;

	AtomGroup *_original = nullptr;
	AtomGroup *_originalAndMates = nullptr;
	AtomGroup *_extraHydrogens = nullptr;
	std::vector<hnet::Coordinated *> _hCoords;
	
	UndoStack _undoStack{};
};

#endif
