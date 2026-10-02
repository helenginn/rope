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

#include "Energy.h"
#include "matrix_functions.h"
#include <array>

using namespace hnet;

Energy::Energy()
{
	_sources[Torsion] = false;
	_sources[Acceptor] = false;
	_sources[Bulk] = false;
	_sources[Distance] = false;
	_sources[Angle] = false;
	_sources[Repulsion] = false;
	_sources[Protonation] = false;

	// seeded to exp(0) = 1 up front for every source, not just the ones
	// with a GUI slider (Repulsion has none - see OccupanciesView::setup())
	// - modulate() takes a pointer via _expAmplifiers[src], and operator[]
	// on a map default-constructs a missing key to 0.f, not 1.f, which
	// would silently zero out that source's contribution instead of
	// leaving it unamplified until alter_amplification() is ever called.
	for (const Source &src : {Torsion, Acceptor, Distance, Angle, Bulk,
	                          Repulsion, Protonation})
	{
		_expAmplifiers[src] = 1.f;
	}
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_covalent(BondProbe &bp)
{
	Probe &left = bp.left();
	Probe &right = bp.right();
	
	auto acquire_connected_atoms_for = [&bp](Probe &probe)
	{
		std::vector<Probe *> connected;
		for (Probe *const &other : probe.others())
		{
			if (!other->is_covalent())
			{
				continue;
			}
			BondProbe *bOther = static_cast<BondProbe *>(other);
			if (bOther == &bp)
			{
				continue;
			}
			if (bOther->existence().value() != Existence::Present)
			{
				continue;
			}

			if (&bOther->left() == &probe)
			{
				connected.push_back(&bOther->right());
			}
			else 
			{
				connected.push_back(&bOther->left());
			}
		}
		return connected;
	};

	auto func = [&bp, &left, &right, acquire_connected_atoms_for]
	()
	{
		if (bp.existence().value() != Existence::Present)
		{
			return 0.f;
		}
		std::vector<Probe *> lefts = acquire_connected_atoms_for(left);

		std::vector<Probe *> rights = acquire_connected_atoms_for(right);

		glm::vec3 l = left.position();
		glm::vec3 r = right.position();
		float sum = 0;

		// conventional (AMBER/CHARMM-style) periodic torsion term, one per
		// (extrLeft, extrRight) pair rather than one abstract dihedral for
		// the whole bond - naturally scales with how many substituents
		// each end has, since a more substituted centre sums over more
		// pairs. n=3 matches a tetrahedral (sp3) centre's own three-fold
		// symmetry - staggered minima every 120 degrees (60, 180, -60),
		// eclipsed maxima every 120 degrees starting at 0. cos(n*theta)/2
		// keeps this centred on zero (mean 0 over a full cycle, favourable
		// staggered contributions negative, unfavourable eclipsed ones
		// positive) rather than shifted up into [0, 1] - and, unlike the
		// old |torsion| > 60 cutoff, is smooth and defined over the full
		// cycle, not just a window around one minimum.
		for (Probe *const &extrLeft : lefts)
		{
			glm::vec3 ll = extrLeft->position();
			bool isHL = (extrLeft->atom()->elementSymbol() == "H");

			for (Probe *const &extrRight : rights)
			{
				glm::vec3 rr = extrRight->position();
				glm::vec3 poz[] = {ll, l, r, rr};
				bool isHR = (extrRight->atom()->elementSymbol() == "H");
				float torsion = measure_bond_torsion(poz);

				float x = deg2rad(torsion * 3.f);
				float term = 0.5f * cos(x);
				if (isHL) term *= 0.5;
				if (isHR) term *= 0.5;
				sum += term;
			}
		}

		float base = 0.5;

		return sum * base;
	};
	return modulate({{func, Torsion}});
}

auto evaluate_hbond(BondProbe &bp)
{
	return [&bp](const hnet::Bond::Values &expect = hnet::Bond::Acceptor,
	             bool only = true)
	{
		hnet::Existence::Values ex = bp._exist.value();

		if (ex == hnet::Existence::Absent && only)
		{
			return false;
		}

		hnet::Bond::Values val = bp._obj.value();
		if (only && (int)val == (int)expect)
		{
			return true;
		}
		else if (!only && val & expect)
		{
			return true;
		}
		return false;
	};
}

float Energy::hbond_angle_acceptance(const glm::vec3 &centre,
                                     const glm::vec3 &hydrogen,
                                     const glm::vec3 &other)
{
	glm::vec3 hl = glm::normalize(hydrogen - centre);
	glm::vec3 rl = glm::normalize(other - centre);

	// clamped: for two (near) identical directions - exactly the ideal,
	// collinear geometry - rounding in the normalisation can leave the dot
	// product a hair over 1, where acos() is NaN, and a NaN energy is
	// silently dropped by modulate(), i.e. no stabilisation at all at the
	// best geometries (about one collinear case in five in float).
	float dot = std::min(1.f, std::max(-1.f, glm::dot(hl, rl)));
	float angle = acos(dot);
	float exag = angle * 3;
	return std::max(cos(exag), 0.f);
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_hbond_angle(HydrogenProbe *probe,
                                       BondProbe &left, BondProbe &right,
                                       AtomProbe &lAtom, AtomProbe &rAtom)
{
	struct AngleCache
	{
		GuiltVersion seenAt = -2;
		float energy = 0.f;
	};

	// one cache slot per side - left/right evaluate different bond/atom
	// pairs even though they share the same hydrogen. shared_ptr so every
	// copy of the returned EnergyWrapper (std::function) reuses the same
	// cache instance rather than each getting its own independent copy
	// (same reasoning as energy_wrapper_for_clash_repulsion's sharedPairs).
	auto lCache = std::make_shared<AngleCache>();
	auto rCache = std::make_shared<AngleCache>();

	auto hbond_angle_for_bond = [probe](BondProbe &bp, AtomProbe &centre,
	                                     AtomProbe &other,
	                                     const std::shared_ptr<AngleCache> &cache)
	{
		auto evaluate = evaluate_hbond(bp);
		if (!evaluate(hnet::Bond::Acceptor, true))
		{
			return 0.f;
		}

		centre.realign();
		other.realign();

		// probe->_lastPositioned only changes when realignment actually
		// repositions this hydrogen (Coordinated_Realign.cpp's
		// transplant_positions()) - realign() above runs every call
		// regardless (it's cheap and does its own internal no-op check),
		// but if the hydrogen hasn't actually moved since this cache slot
		// last computed, the angle can't have changed either, so skip the
		// trig and reuse the cached contribution instead of recomputing it.
		if (cache->seenAt == probe->_lastPositioned)
		{
			return cache->energy;
		}

		glm::vec3 lPos = centre.position();
		glm::vec3 rPos = other.position();
		glm::vec3 hPos = probe->_h->initialPosition();

		float damping = hbond_angle_acceptance(lPos, hPos, rPos);

		cache->energy = 1.0f * -damping;
		cache->seenAt = probe->_lastPositioned;
		return cache->energy;
	};

	auto hbond_angle = [&, hbond_angle_for_bond, lCache, rCache]()
	{
		float total = 0;
		total += hbond_angle_for_bond(left, lAtom, rAtom, lCache);
		total += hbond_angle_for_bond(right, rAtom, lAtom, rCache);
		return total;
	};

	return modulate({{hbond_angle, Angle}});
}

float Energy::hbond_electrostatic_sum(const glm::vec3 &donor,
                                      const glm::vec3 &hydrogen,
                                      const glm::vec3 &acceptor,
                                      const std::vector<ChargedPosition>
                                      &parents, float donorFactor,
                                      float hydrogenFactor,
                                      float acceptorFactor)
{
	// 1 / distance, with a floor so two coincident points can't blow up the
	// sum (real contacts are never anywhere near this close).
	auto inverse = [](const glm::vec3 &a, const glm::vec3 &b)
	{
		float dist = std::max(glm::length(a - b), 0.5f);
		return 1.f / dist;
	};

	// charges: donor -d, hydrogen +d, acceptor -d, acceptor parents +d
	// (shared out by weight) - in units of d^2, so each pair's sign is
	// simply the product of the two signs:
	//  donor - acceptor                 (-)(-) = +  repulsion
	//  hydrogen - acceptor              (+)(-) = -  attraction
	//  hydrogen - each parent (share w) (+)(+) = +  repulsion
	//  donor - each parent (share w)    (-)(+) = -  attraction

	// the two atoms' own charges are there whether or not there is a
	// hydrogen between them. The pair appears in both half-bonds (each takes
	// the other as its donor), so each carries half of it.
	float sum = 0.5f * donorFactor * acceptorFactor * inverse(donor, acceptor);

	for (const ChargedPosition &parent : parents)
	{
		sum -= donorFactor * acceptorFactor * parent.weight *
		inverse(donor, parent.pos);
	}

	// the hydrogen's own charge only counts if it is the donor's hydrogen -
	// nearer the donor than the acceptor. A hydrogen nearer the acceptor is
	// the acceptor's own (bonded to it: no electrostatic interaction), and
	// one equidistant from both hasn't been placed on either.
	bool donors = glm::length(hydrogen - donor) <
	glm::length(hydrogen - acceptor);

	if (hydrogenFactor > 0 && donors)
	{
		sum -= hydrogenFactor * acceptorFactor * inverse(hydrogen, acceptor);

		for (const ChargedPosition &parent : parents)
		{
			sum += hydrogenFactor * acceptorFactor * parent.weight *
			inverse(hydrogen, parent.pos);
		}
	}

	return sum;
}

// how certainly something exists: 1 = present, 0 = absent, in between = not
// yet settled.
static float existence_factor(hnet::ExistenceConnector &connector)
{
	hnet::Existence::Values value = connector.value();
	if (value == hnet::Existence::Absent)
	{
		return 0.f;
	}
	if (value == hnet::Existence::Present)
	{
		return 1.f;
	}
	return 0.5f;
}

// a hydrogen is only there if both its protonation state (h) and its
// existence (hExist) say so - either can settle Absent on its own (see
// HydrogenProbe::display()).
static float hydrogen_factor(HydrogenProbe &probe)
{
	float protonation = existence_factor(probe._obj);
	float existence = existence_factor(probe.existence());
	return std::min(protonation, existence);
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_half_hbond(HydrogenProbe *probe, BondProbe &bp,
                                      AtomProbe &donor,
                                      const glm::vec3 &donorPos,
                                      AtomProbe &acceptor,
                                      const glm::vec3 &acceptorPos,
                                      const std::vector<ChargedPosition>
                                      &acceptorParents)
{
	auto evaluate = evaluate_hbond(bp);

	auto hbond_base = [evaluate, &bp]()
	{
		float base = 1.0;
		if (evaluate(hnet::Bond::Acceptor, true))
		{
			float contrib = -base;
			return contrib;
		}
		else if (evaluate(hnet::Bond::Acceptor, false))
		{
			float contrib = -base;
			if (bp.existence().value() & hnet::Existence::Absent)
			{
				contrib /= 2.f;
			}
			return contrib;
		}
		return -0.5f;
	};

	// Electrostatics, deliberately NOT gated on this half-bond's own state
	// (Acceptor / LonePair / Broken ...): unlike hbond_base above and the
	// angle term, which are the bond's partial covalent character and exist
	// only where the bond does, charges interact regardless. Only the
	// existence of the atoms and the hydrogen matters.
	auto hbond_dist = [probe, &donor, &acceptor, donorPos, acceptorPos,
	                  acceptorParents]()
	{
		float fD = existence_factor(donor.existence());
		float fA = existence_factor(acceptor.existence());
		float fH = hydrogen_factor(*probe);

		if (fA == 0.f || (fD == 0.f && fH == 0.f))
		{
			return 0.f;
		}

		// where the hydrogen is depends on the bond pattern around its
		// donor, which realign() brings up to date (a no-op if nothing has
		// changed). Nothing else in this energy ever calls it - the angle
		// term does, but only for a bond settled as Acceptor, and in
		// whatever order the energy terms happen to be collected - so
		// without this the hydrogen could be read where an earlier
		// configuration left it. Either atom may be the one that owns it.
		glm::vec3 hPos = acceptorPos;
		if (fH > 0.f)
		{
			donor.realign();
			acceptor.realign();
			hPos = probe->position();

			if (hPos.x != hPos.x || hPos.y != hPos.y || hPos.z != hPos.z)
			{
				return 0.f;
			}
		}

		float sum = hbond_electrostatic_sum(donorPos, hPos, acceptorPos,
		                                    acceptorParents, fD, fH, fA);

		return HBondElectrostatic * HBondDeltaCharge * HBondDeltaCharge * sum;
	};

	return modulate({{hbond_base, Acceptor}, {hbond_dist, Distance}});
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_liberated_bulk(AtomProbe &bulk)
{
	ExistenceConnector &bulkExist = bulk.existence();

	auto func = [&bulkExist]()
	{
		float base = 1.0;
		hnet::Existence::Values ex = bulkExist.value();

		if (ex == hnet::Existence::Absent)
		{
			return 0.f;
		}

		// bulk solvent will provide 3.6 hydrogen bonds on average
		float contrib = -base * 3.6;
		return contrib;
	};
	
	return modulate({{func, Bulk}});
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_clash_repulsion(
    ExistenceConnector &groupExist,
    const std::vector<Probe::PendingRepulsion> &pairs,
    std::shared_ptr<GuiltVersion> lastRound)
{
	// shared, not copied per group member - std::function's own copy
	// semantics duplicate whatever a lambda captures by value, so
	// capturing pairs directly here would give every member of an
	// N-atom group its own full copy of the N-sized array (O(N^2)
	// memory for a large alt-conf cluster, likely the actual driver
	// behind memory still climbing during exhaustive search) once the
	// identical wrapper is attached to each member via
	// Probe::addEnergyWrapper() in Network::bundleRepulsionTerms().
	auto sharedPairs =
	std::make_shared<std::vector<Probe::PendingRepulsion>>(pairs);

	auto func = [&groupExist, sharedPairs]()
	{
		// every pair's own "left" atom belongs to this same mutual-
		// existence group, so their existence is guaranteed correlated -
		// checked once here (via any representative member's connector)
		// instead of once per pair, short-circuiting the whole sum
		// immediately if the group is absent.
		if (groupExist.value() != hnet::Existence::Present)
		{
			return 0.f;
		}

		// sums every pair's own existence-gated contribution, rather than
		// picking a single "worst" one - a mutual-existence group's
		// members can each genuinely be clashing with a different
		// neighbour at once, and none of that should be silently dropped
		// just because they now share one wrapper. Only targetExist -
		// unrelated between pairs, unlike selfExist above - still needs
		// checking individually.
		float total = 0.f;

		for (const Probe::PendingRepulsion &p : *sharedPairs)
		{
			if (p.dist <= 0.f || p.dist != p.dist || p.sigma <= 0.f)
			{
				continue;
			}

			if (!p.targetExist ||
			    p.targetExist->value() != hnet::Existence::Present)
			{
				continue;
			}

			// repulsive half only of the usual 12-6 Lennard-Jones form
			// (see BundleBonds.cpp's own vdw_energy() for the full
			// version with the attractive term, once that gets added
			// here too) - sigma is the sum of the pair's real van der
			// Waals radii (gemmi), so this is epsilon right at the point
			// the two atoms nominally touch, climbing steeply inside it
			// and negligible beyond. Positive (a cost), unlike the
			// negative/favourable contributions elsewhere in this file.
			float ratio = p.sigma / p.dist;
			float r2 = ratio * ratio;
			float r4 = r2 * r2;
			float r6 = r4 * r2;
			float r12 = r6 * r6;
			total += p.epsilon * r12;
		}

		return total;
	};

	hnet::EnergyWrapper inner = modulate({{func, Repulsion}});

	// lastRound is shared across the whole group (not per pair), so a
	// scoring round only counts this combined term once even if several
	// group members end up in the same _wider set - GuiltVersion
	// doubling as a round id here, not a blame tag (see EnergyWrapper's
	// own comment).
	return [inner, lastRound](GuiltVersion gv) -> hnet::GetEnergy
	{
		if (lastRound)
		{
			if (*lastRound == gv)
			{
				return {};
			}
			*lastRound = gv;
		}

		return inner(gv);
	};
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_protonation(CountConnector &charge,
                                       ExistenceConnector &existence,
                                       Count::Values deprotonated,
                                       const std::function<float()> &getDeltaG)
{
	return [this, &charge, &existence, 
	        deprotonated, getDeltaG](GuiltVersion) -> hnet::GetEnergy
	{
		// an absent atom/group's charge is meaningless (never resolved to
		// one value - see Network::applyPKaEnergy()'s own comment on
		// CountProbe::certainValueAsInt()), so existence must be checked
		// directly rather than folded into the deprotonated comparison
		// below - reading charge.value() at all while absent is what
		// broke the pH slider before this fix.
		if (existence.value() == hnet::Existence::Absent)
		{
			return []() { return 0.f; };
		}
		// charge's own value is read HERE, once per GuiltVersion - same
		// contract modulate() gives every other energy source.
		// ExhaustiveSearch::score_wider_clique() calls Probe::energy(gv)
		// exactly once per candidate state, with charge holding that
		// candidate's own value at the time; reading it again later, from
		// inside the returned GetEnergy, would just see charge's current
		// resting value - the same one regardless of which state i this
		// GetScore closure is nominally scoring - which would make this
		// term a constant added to every state alike, cancelling out of
		// probsForAve()'s (ave - score(i)) entirely and silently
		// producing no effect at all.
		bool isDeprotonated = (charge.value() == deprotonated);
		
		// getDeltaG()/source_on()/amplification(), unlike charge's own
		// value above, deliberately do stay live - re-evaluated on every
		// call to the returned GetEnergy, not just once here - see this
		// function's own header comment for why (Network::setTestPH()
		// needs to affect results even from an already-cached GetScore).
		return [this, isDeprotonated, getDeltaG]() -> float
		{
			if (!isDeprotonated)
			{
				return 0.f;
			}
			if (!source_on(Protonation))
			{
				return 0.f;
			}
			return getDeltaG() * expAmplification(Protonation);
		};
	};
}

hnet::EnergyWrapper
Energy::modulate(const std::vector<SourcedEnergy> &sources)
{
	// gv (the current scoring round) is unused here - modulate() combines
	// per-source contributions the same way regardless of round; only
	// energy_wrapper_for_clash_repulsion's own memo (wrapped around this
	// function's result) cares which round it's being called in.
	return [this, sources](GuiltVersion gv) -> hnet::GetEnergy
	{
		struct Cached
		{
			bool *ptr;
			float *expAmp;
			float energy;
		};

		// every call site above passes at most 2 sources - a fixed inline
		// buffer instead of a std::vector avoids a heap alloc/free on
		// every single evaluation of this closure, which runs once per
		// accepted config x per wider probe in ExhaustiveSearch (the
		// dominant source of the "freeing/memsetting" time seen in
		// profiles of Energy::modulate).
		constexpr size_t MaxSources = 4;
		std::array<Cached, MaxSources> cache;
		size_t count = 0;

		for (const SourcedEnergy &se : sources)
		{
			float cached_energy = se.first();
			if (fabs(cached_energy) < 1e-6)
			{
				continue;
			}
			if (cached_energy != cached_energy)
			{
				continue;
			}
			bool *source_ptr = &_sources[se.second];
			// exp(amp) is precomputed by alter_amplification() (only run
			// when a GUI slider actually moves) rather than here - this
			// closure is replayed on every CertainStates::probsForLocalAve()
			// call, far more often than the amplifier itself ever changes,
			// so calling exp() on every replay was pure waste.
			float *exp_amp_ptr = &_expAmplifiers[se.second];
			if (count < MaxSources)
			{
				cache[count++] = {source_ptr, exp_amp_ptr, cached_energy};
			}
		}

		if (count == 0) return {};

		return [this, cache, count]()
		{
			float total = 0;
			for (size_t i = 0; i < count; i++)
			{
				if (*cache[i].ptr)
				{
					total += cache[i].energy * (*cache[i].expAmp);
				}
			}

			return total;
		};
	};
}
