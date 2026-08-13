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

using namespace hnet;

Energy::Energy()
{
	_sources[Torsion] = false;
	_sources[Acceptor] = false;
	_sources[Bulk] = false;
	_sources[Distance] = false;
	_sources[Angle] = false;
	_sources[Repulsion] = false;
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

		return sum;
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
		glm::vec3 hl = glm::normalize(hPos - lPos);
		glm::vec3 rl = glm::normalize(rPos - lPos);
		float dot = glm::dot(hl, rl);
		float angle = acos(dot);
		float exag = angle * 3;
		float damping = std::max(cos(exag), 0.f);

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

hnet::EnergyWrapper
Energy::energy_wrapper_for_half_hbond(HydrogenProbe *probe, BondProbe &bp, 
                                      glm::vec3 pos)
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
	};

	auto hbond_dist = [evaluate, probe, pos]()
	{
		if (!evaluate(hnet::Bond::Acceptor, false))
		{
			return 0.f;
		}

		glm::vec3 hPos = probe->position();
		float dist = glm::length(hPos - pos);
		if (dist != dist)
		{
			return 0.f;
		}

		float base = 1.0;
		float mod = 1 / (dist * dist);
		float contrib = -base * mod;
		if (!evaluate(hnet::Bond::Acceptor, true))
		{
			contrib /= 2;
		}
		return contrib;
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
			float *amp;
			float energy;
		};

		std::vector<Cached> cache;

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
				cached_energy = 0.f;
			}
			bool *source_ptr = &_sources[se.second];
			float *amp_ptr = &_amplifiers[se.second];
			cache.push_back({source_ptr, amp_ptr, cached_energy});
		}
		
		if (cache.size() == 0) return {};
		
		return [this, cache]()
		{
			float total = 0;
			for (const auto &entry : cache)
			{
				if (*entry.ptr)
				{
					total += entry.energy * exp(*entry.amp);
				}
				if (total != total)
				{
					std::cout << "Entry: " << *entry.ptr << " " << *entry.amp << " " <<
					entry.energy << std::endl;
				}
			}

			return total;
		};
	};
}
