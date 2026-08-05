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

#ifndef __vagabond__Energy__
#define __vagabond__Energy__

#include "hnet.h"
#include "Probe.h"
#include <map>
#include <memory>

namespace hnet
{
	// GetEnergy/EnergyWrapper already declared by Probe.h (included above)

class Energy
{
public:
	Energy();

	enum Source
	{
		Torsion,
		Acceptor,
		Distance,
		Angle,
		Bulk,
		Repulsion,
		Unknown,
	};

	void alter_amplification(const Source &src, float amp)
	{
		_amplifiers[src] = amp;
	}

	float amplification(const Source &src)
	{
		return _amplifiers.count(src) ? _amplifiers.at(src) : 0.f;
	}

	void alter_source(const Source &src, bool on)
	{
		_sources[src] = on;
	}

	bool source_on(const Source &src)
	{
		return _sources[src];
	}

	EnergyWrapper energy_wrapper_for_covalent(BondProbe &bp);

	EnergyWrapper energy_wrapper_for_half_hbond(HydrogenProbe *probe, 
	                                            BondProbe &bp, glm::vec3 pos);

	EnergyWrapper energy_wrapper_for_liberated_bulk(AtomProbe &bulk);

	EnergyWrapper energy_wrapper_for_hbond_angle(HydrogenProbe *probe,
	                                             BondProbe &left,
	                                             BondProbe &right,
	                                             AtomProbe &lAtom,
	                                             AtomProbe &rAtom);

	/** soft steric repulsion, one shared wrapper covering an entire
	 * mutual-existence-connected group's worth of {left, right} clash
	 * candidates (Network::bundleRepulsionTerms()) rather than one
	 * wrapper per atom - every pair's own existence-gated contribution is
	 * summed internally each time this is evaluated, so no group
	 * member's genuine clash gets silently dropped the way picking only
	 * the single closest one would. dist/sigma/epsilon per pair are
	 * captured by value at clashLogic() setup time, since atom positions
	 * do not move once a Network is built. Repulsive term only (no
	 * London dispersion/attractive term yet - deliberately deferred).
	 * groupExist is checked once, not once per pair - every pair's own
	 * "left" atom belongs to the same mutual-existence group, so their
	 * existence is guaranteed correlated (any representative member's
	 * connector will do), letting the whole sum short-circuit
	 * immediately if the group is absent instead of redundantly
	 * rechecking the same fact once per pair. Each pair's own
	 * targetExist is unrelated to the others and still checked
	 * individually. The identical wrapper is handed to every member of
	 * the group (see Network::bundleRepulsionTerms()); lastRound (shared
	 * across the whole group, not per pair) makes sure a scoring round
	 * only counts it once even if several members end up in the same
	 * _wider set. */
	EnergyWrapper energy_wrapper_for_clash_repulsion(
	    ExistenceConnector &groupExist,
	    const std::vector<Probe::PendingRepulsion> &pairs,
	    std::shared_ptr<GuiltVersion> lastRound);
private:
	
	typedef std::pair<GetEnergy, Source> SourcedEnergy;

	EnergyWrapper modulate(const std::vector<SourcedEnergy> &sources);

	std::map<Source, bool> _sources;
	std::map<Source, float> _amplifiers;
};
};

#endif
