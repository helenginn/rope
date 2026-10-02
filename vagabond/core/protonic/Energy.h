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
		Protonation,
		Unknown,
	};

	void alter_amplification(const Source &src, float amp)
	{
		_amplifiers[src] = amp;
		// precomputed here, where this only runs when a GUI slider actually
		// moves - modulate()'s inner closure (and energy_wrapper_for_
		// protonation) both run this exp() on every single scoring call
		// otherwise, including every replay of an already-cached GetScore
		// during CertainStates::probsForLocalAve(), which is by far the
		// hotter path.
		_expAmplifiers[src] = exp(amp);
	}

	float amplification(const Source &src)
	{
		return _amplifiers.count(src) ? _amplifiers.at(src) : 0.f;
	}

	float expAmplification(const Source &src)
	{
		return _expAmplifiers.count(src) ? _expAmplifiers.at(src) : 1.f;
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

	/** a position carrying a share of a charge - see
	 *  energy_wrapper_for_half_hbond(). */
	struct ChargedPosition
	{
		glm::vec3 pos{};
		float weight = 1.f; // this position's share of the total charge
	};

	/** constant partial charge (in units of e) on each end of the two
	 *  dipoles taking part in an H-bond: -delta on the donor, +delta on its
	 *  hydrogen, -delta on the acceptor, +delta shared equally among the
	 *  acceptor's covalent parents. Not fitted to anything - it and the
	 *  constant below only set the overall size of the electrostatic term
	 *  (also scaled by the "Distance" slider). */
	static constexpr float HBondDeltaCharge = 0.4f;

	/** Coulomb-like prefactor: energy = HBondElectrostatic * q1 * q2 / d
	 *  (d in angstroms), in this class's own arbitrary energy units.
	 *  Calibrated on what actually decides state weights - the DIFFERENCE
	 *  between the H-bonded and not-H-bonded state of a pair - so that at an
	 *  ideal linear amide N-H...O=C contact (H...O 1.9 A) forming the
	 *  H-bond is worth about -0.14, as the hydrogen-acceptor term this
	 *  replaced was. (The absolute value now also includes the always-on
	 *  donor-acceptor terms, and is larger than that.) */
	static constexpr float HBondElectrostatic = 4.5f;

	/** The electrostatic terms of one half of an H-bond (see
	 *  energy_wrapper_for_half_hbond()), in units of HBondElectrostatic *
	 *  HBondDeltaCharge^2 - just the geometry: the sum of 1/distance over
	 *  each pair, signed by the product of the two charges' signs, so
	 *  negative means net attraction. Distances are floored at 0.5 A. A pure
	 *  function of positions, so it can be tested on its own.
	 *
	 *  Each term is weighted by how certainly the atoms it involves exist
	 *  (1 = present, 0 = absent, in between = not yet settled):
	 *   - donor ... acceptor, repulsion (-,-): donorFactor * acceptorFactor,
	 *     and only HALF of it, because the mirror-image half-bond (the same
	 *     two atoms with the roles swapped) carries the other half;
	 *   - donor ... acceptor's parents, attraction (-,+): donorFactor *
	 *     acceptorFactor;
	 *   - hydrogen ... acceptor, attraction (+,-) and hydrogen ... acceptor's
	 *     parents, repulsion (+,+): hydrogenFactor * acceptorFactor, and only
	 *     if the hydrogen is nearer the donor than the acceptor - i.e. it is
	 *     the donor's hydrogen. Otherwise it is the other atom's own
	 *     hydrogen (or not yet placed anywhere in particular), and its
	 *     interaction with the atom it is covalently bonded to is not an
	 *     electrostatic one at all.
	 *  None of it depends on what state either half-bond has settled on:
	 *  charges attract and repel whether or not the bond is formed. */
	static float hbond_electrostatic_sum(const glm::vec3 &donor,
	                                     const glm::vec3 &hydrogen,
	                                     const glm::vec3 &acceptor,
	                                     const std::vector<ChargedPosition>
	                                     &parents, float donorFactor,
	                                     float hydrogenFactor,
	                                     float acceptorFactor);

	/** electrostatics of one half of an H-bond: the half whose atom (`acceptor`)
	 *  is treated as the acceptor, against the atom on the other side
	 *  (`donor`) and the hydrogen between them - four Coulomb terms between
	 *  the donor/hydrogen dipole and the acceptor/parents dipole, with
	 *  charges as described at HBondDeltaCharge:
	 *    donor ... acceptor            repulsion  (-,-)
	 *    hydrogen ... acceptor         attraction (+,-)
	 *    hydrogen ... acceptor parents repulsion  (+,+)
	 *    donor ... acceptor parents    attraction (-,+)
	 *  Evaluated whatever bond state the half-bond has settled on (not only
	 *  Acceptor): it depends only on which atoms and hydrogen exist - see
	 *  hbond_electrostatic_sum(). The hydrogen's position is read live, after
	 *  both heavy atoms have been realigned (a no-op if the bonds around them
	 *  haven't changed) so that it is where the current bond pattern puts it;
	 *  the heavy atoms' positions are fixed once a Network is built, so the
	 *  caller identifies the acceptor's parents (its covalently bonded
	 *  partners, matched per conformer by the network's covalent bonds) when
	 *  this is made, and passes them in with their shares of the +delta
	 *  charge (summing to 1).
	 *  @param donor the other heavy atom, whose hydrogen this is.
	 *  @param acceptor this half-bond's own heavy atom. */
	EnergyWrapper energy_wrapper_for_half_hbond(HydrogenProbe *probe,
	                                            BondProbe &bp,
	                                            AtomProbe &donor,
	                                            const glm::vec3 &donorPos,
	                                            AtomProbe &acceptor,
	                                            const glm::vec3 &acceptorPos,
	                                            const std::vector<ChargedPosition>
	                                            &acceptorParents);

	EnergyWrapper energy_wrapper_for_liberated_bulk(AtomProbe &bulk);

	/** how far out of line (degrees) the donor-hydrogen...acceptor chain may
	 *  bend before the hydrogen bond's "covalent character" stabilisation is
	 *  lost completely - see hbond_angle_acceptance(). */
	static constexpr float HBondAngleCutoff = 30.f;

	/** how fully the hydrogen-bond "covalent character" stabilisation applies,
	 *  0 to 1, for the chain donor - hydrogen ... acceptor. It depends on how
	 *  far the chain is bent out of a straight line, i.e. 180 degrees minus
	 *  the D-H...A angle measured AT the hydrogen: 1 when perfectly linear,
	 *  falling as cos(90 degrees * bend / HBondAngleCutoff) to exactly 0 at a
	 *  bend of HBondAngleCutoff (30 degrees, a D-H...A angle of 150), and 0
	 *  beyond that. Pure function of positions (see
	 *  energy_wrapper_for_hbond_angle()). Returns 0 if the hydrogen coincides
	 *  with either atom, as there is then no angle to speak of. */
	static float hbond_angle_acceptance(const glm::vec3 &donor,
	                                    const glm::vec3 &hydrogen,
	                                    const glm::vec3 &acceptor);

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

	/** turns a live ΔG getter (kJ/mol, deprotonated relative to protonated
	 * - see CustomProtonSettings::deprotonationEnergyFor()) into a state-
	 * dependent energy term: 0 while charge's own resolved value equals
	 * deprotonated, getDeltaG() otherwise. Deliberately generic (knows
	 * nothing about pH/pKa itself, or which residue this is) - the caller
	 * has already reduced the whole Henderson-Hasselbalch relationship
	 * down to a callback. Once CertainStates' own Boltzmann weighting
	 * (score() -> probsForAve(), same physiological_rt_kjmol - see
	 * maths.h) is applied across a fully-enumerated state set, the
	 * resulting probability ratio between the two states reproduces
	 * deprotonated:protonated = 10^(pH-pKa), independent of whatever
	 * baseline the caller chose for the protonated side, since only the
	 * *difference* the Boltzmann factor sees here ever matters.
	 *
	 * Deliberately bypasses modulate() (unlike every other energy source
	 * here) - modulate() evaluates each source's raw function once per
	 * GuiltVersion and bakes the result into the GetEnergy closure it
	 * returns, which is exactly right for sources that only change
	 * between scoring rounds, but wrong here: Network::setTestPH() (see
	 * its own comment) needs a pH nudge to be reflected even in results
	 * that reuse an already-scored GetScore/GetEnergy from a finished
	 * exhaustive search (CertainStates::score() calling the same cached
	 * closure repeatedly, long after the scoring round it was built in).
	 * So this evaluates getDeltaG(), source_on(Protonation) and
	 * amplification(Protonation) fresh on every single call instead. */
	EnergyWrapper energy_wrapper_for_protonation(CountConnector &charge,
	                                             ExistenceConnector &existence,
	                                             Count::Values deprotonated,
	                                             const std::function<float()> &getDeltaG);
private:
	
	typedef std::pair<GetEnergy, Source> SourcedEnergy;

	EnergyWrapper modulate(const std::vector<SourcedEnergy> &sources);

	std::map<Source, bool> _sources;
	std::map<Source, float> _amplifiers;
	std::map<Source, float> _expAmplifiers;
};
};

#endif
