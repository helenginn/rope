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

class CountProbe;
class CovalentProbe;
class Model;
class Progressor;

namespace hnet
{
	class Coordinated;
	class Energy;
}

class Network
{
public:
	Network();
	~Network();

	// progress, if given, gets one clickTicker() per major phase of
	// construction (~15 - see the ASCII banners inside the constructor
	// for the phase boundaries) and a finishTicker() right at the end -
	// coarse, but this constructor is one long unbroken sweep with no
	// natural per-atom yielding point, so phase-level is the cheapest
	// granularity that still gives a genuinely moving progress bar. Safe
	// to run this constructor itself off the main thread (see
	// ProtonNetworkView's own build/rebuild methods) - nothing in here
	// touches GL or other main-thread-only state, only AtomGroup/Atom
	// (core) and Probe/Connector (also core, no rendering).
	Network(AtomGroup *group, const std::string &spg_name,
	        const std::array<double, 6> &unit_cell = {},
	        Model *const &model = {}, Progressor *progress = nullptr);

	template <class Connector>
	auto &add(Connector *const &connector)
	{
		_connectors.push_back(hnet::AnyConnector(connector));
		return *connector;
	}

	template <class Constraint>
	auto &add_constraint(Constraint *const &constraint)
	{
		_constraints.push_back(constraint);
		return *constraint;
	}
	
	const std::list<HydrogenProbe *> &hydrogenProbes() const
	{
		return _hydrogenProbes;
	}

	// every AtomConf ever passed to addNewHydrogen() (bonded or
	// placeholder), maintained incrementally there rather than rebuilt
	// from _hAtoms on demand - lets Coordinated::findNeighbours() reuse
	// the same cheap bounding-box-filtered search clashLogic() already
	// uses, without paying an O(n) AtomGroup->OpSet rebuild on every
	// single placeholder hydrogen created.
	const OpSet<hnet::AtomConf> &hydrogenAtomConfs() const
	{
		return _hydrogenAtomConfs;
	}

	const std::list<BondProbe *> &bondProbes() const
	{
		return _bondProbes;
	}
	
	const std::list<AtomProbe *> &atomProbes() const
	{
		return _atomProbes;
	}
	
	const std::list<CountProbe *> &chargeProbes() const
	{
		return _chargeProbes;
	}
	
	UndoStack &undoStack()
	{
		return _undoStack;
	}
	
	glm::vec3 centre() const;
	
	AtomProbe &add_probe(AtomProbe *const &probe);
	BondProbe &add_probe(BondProbe *const &probe);
	CountProbe &add_probe(CountProbe *const &probe, bool charge = false);
	HydrogenProbe &add_probe(HydrogenProbe *const &probe, const char &conf);
	
	void addNewHydrogen(hnet::AtomConf hydrogen, 
	                    hnet::ExistenceConnector &hCombo);
	
	AtomGroup *atoms()
	{
		return _original;
	}

	std::map<hnet::AtomConf, hnet::ExistenceConnector *> &existMap()
	{
		return _existMap;
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
	
	HydrogenProbe *probeForHydrogen(const hnet::AtomConf &ac)
	{
		if (_h2Probe.count(ac))
		{
			return _h2Probe.at(ac);
		}
		return nullptr;
	}
	
	std::vector<CovalentProbe *> covalentsForAtom(const hnet::AtomConf &ac)
	{
		return _atom2Covs[ac];
	}
	
	void addImpromptuCollapse(const std::string &message)
	{
		_impromptuCollapses.push_back(message);
	}
	
	const std::vector<std::string> &impromptuCollapses() const
	{
		return _impromptuCollapses;
	}
	
	Clique *newClique(const OpSet<Probe *> &probes);
	void removeClique(Clique *clique);

	std::list<Clique> &cliques()
	{
		return _cliques;
	}
	
	hnet::Energy &energy()
	{
		return *_energy;
	}

	Model *const &model() const
	{
		return _model;
	}

	void updateModelCliques();
	void firstOrderLogic();
	
	void promptReclique();
	bool _reclique = false;

	AtomGroup *assignCertainHydrogens(std::ostringstream &ss);
private:
	void establishAtom(::Atom *atom);

	/** runs once, after every atom's Coordinated::clashLogic() has had a
	 * turn (each staging at most one candidate via Probe::
	 * setPendingRepulsion() rather than building an EnergyWrapper
	 * directly) - groups atoms by Probe::mutualExistenceNeighbours()
	 * connectivity, and for each connected group with at least one
	 * pending candidate, builds exactly one shared clash-repulsion
	 * wrapper (from whichever member's candidate is closest) and attaches
	 * the identical wrapper to every member of the group. */
	void bundleRepulsionTerms();

	void setupInactiveAtom(hnet::AtomConf atom);
	bool setupAmineNitrogen(hnet::AtomConf atom);
	bool setupGuessLigand(hnet::AtomConf atom);
	bool setupCarbonylOxygen(hnet::AtomConf atom);
	bool setupSingleAlcohol(hnet::AtomConf atom);
	bool setupLysineAmine(hnet::AtomConf atom);
	bool setupWater(hnet::AtomConf atom);
	bool setupSodium(hnet::AtomConf atom);
	bool setupArginine(hnet::AtomConf atom);
	bool setupAsnGlnNitrogen(hnet::AtomConf atom);
	bool setupCarboxylOxygen(hnet::AtomConf atom);
	bool setupHistidine(hnet::AtomConf atom);
	bool setupTryptophan(hnet::AtomConf atom);
	bool setupMethionine(hnet::AtomConf atom);

	void linkCovalentBonds(hnet::AtomConf atom);

	// merges any number (2+) of atoms' charges into one shared total
	// (chained CountAdders, see shareProperty() in Network.cpp), creates
	// the CountProbe that displays/samples it, and marks every atom
	// involved as charge-shared (Coordinated::setChargeShared()) so its
	// own individual charge probe steps aside in favour of this one.
	void shareCharges(const std::vector<hnet::AtomConf> &atoms,
	                  const hnet::Count::Values &allowable);

	void findAtomCoordinations(hnet::AtomConf atom);

	void tick();

	std::list<hnet::AnyConnector> _connectors;
	std::list<hnet::ConstraintBase *> _constraints;
	std::list<AtomProbe *> _atomProbes;
	std::list<HydrogenProbe *> _hydrogenProbes;
	std::list<BondProbe *> _bondProbes;
	std::list<CountProbe *> _countProbes;
	std::list<CountProbe *> _chargeProbes;
	std::list<CovalentProbe *> _covalentProbes;
	std::map<std::string, Probe *> _desc2Probe;
	
	std::list<Clique> _cliques;
	Model *_model{};
	Progressor *_progress{};
	hnet::Energy *_energy{};

	std::vector<std::string> _impromptuCollapses;

	std::map<hnet::AtomConf, hnet::Coordinated *> _atomMap;
	std::map<hnet::AtomConf, hnet::ExistenceConnector *> _existMap;
	std::map<hnet::AtomConf, AtomProbe *> _atom2Probe;
	std::map<hnet::AtomConf, HydrogenProbe *> _h2Probe;
	std::map<hnet::AtomConf, std::vector<CovalentProbe *>> _atom2Covs;
	OpSet<hnet::AtomConf> _hydrogenAtomConfs;

	AtomGroup *_original = nullptr;
	AtomGroup *_originalAndMates = nullptr;
	AtomGroup *_hAtoms = nullptr;
	
	UndoStack _undoStack{};
};

#endif
