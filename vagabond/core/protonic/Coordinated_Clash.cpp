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

#define HYDROGEN_BONDING_TOLERANCE (45.0f)
#define HYDROGEN_MAX_DISTANCE (3.5f)
#define PLANAR_TOLERANCE (45.0f)

#include <iostream>

#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/glm_import.h>
#include "matrix_functions.h"
#include "CovalentProbe.h"
#include "Coordinated.h"
#include "Covalent2Count.h"
#include "OrCount.h"
#include "CountProbe.h"
#include "BondAngle.h"
#include "AtomGroup.h"
#include "Energy.h"
#include "Guilt.h"
#include <gemmi/elem.hpp>

using namespace hnet;

#include "Coordinated_Helpers.h"

// steric clash / van der Waals repulsion logic. See
// Coordinated_Core.cpp for the note on how this class's
// implementation is split across files.

// eyeballed to roughly match published epsilon tables (kcal/mol) - same
// values BundleBonds.cpp's own findCoefficients() already uses for its
// unrelated vdw_energy() term, kept consistent here rather than inventing
// a second table.
static float vdw_epsilon_for_element(const gemmi::Element &ele)
{
	switch (ele.atomic_number())
	{
		case 1: return 0.0037f; // H
		case 6: return 0.0205f; // C
		case 7: return 0.0407f; // N
		case 8: return 0.0502f; // O
		case 16: return 0.0600f; // S
		default: return 0.07f;
	}
}

void Coordinated::clashLogic(OpSet<AtomConf> &clash_check)
{
	// per-atom upper bound rather than one flat distance for every atom -
	// this atom's own van der Waals radius plus the largest radius any
	// realistic partner could have (sulphur, ~1.8 A) - refined per-pair
	// below to each pair's own actual radius sum (gemmi), the real
	// cutoff for the soft repulsion term. A flat search distance generous
	// enough to cover S + S pairs was needlessly wide for every smaller
	// atom (most of them - a hydrogen's own plausible partners top out
	// far below that), pulling in far more neighbours than the repulsion
	// term could ever actually use and slowing this down a lot.
	const float MAX_PARTNER_VDW_RADIUS = 1.85f; // sulphur
	gemmi::Element selfEle(_atomConf.ptr->elementSymbol());
	float searchDistance = selfEle.vdw_r() + MAX_PARTNER_VDW_RADIUS;

	OpSet<AtomConf> hits = findNeighbours(clash_check, atomic_position(),
	                                      searchDistance, false);

	auto is_twirling_hydrogen = [this](const ::Atom *q)
	{
		if (q->elementSymbol() != "H" || q->bondLengthCount() != 1)
		{
			return false;
		}

		::Atom *p = q->connectedAtom(0);

		int hcount = 0;
		for (int i = 0; i < p->bondLengthCount(); i++)
		{
			if (p->connectedAtom(i)->elementSymbol() == "H")
			{
				hcount++;
			}
		}

		return (hcount == 3);
	};

	// this atom's own single worst (closest) offender only, not one term
	// per neighbour in range - keeping every candidate was adding far
	// more energy terms than this could comfortably afford. A
	// simplification to build on later, not a final model.
	AtomConf closestCandidate{};
	ExistenceConnector *closestRight = nullptr;
	float closestDist = FLT_MAX;
	bool haveCandidate = false;

	// sigma is the sum of the pair's actual van der Waals radii - the
	// real distance a full (attraction included) Lennard-Jones potential
	// would settle at, and so the sensible maximum distance for a
	// repulsion-only term to reach out to, even though we're not yet
	// adding the attractive London dispersion half.
	auto add_repulsion = [this, &selfEle](const AtomConf &hit, float dist,
	                                      ExistenceConnector *rightExist)
	{
		gemmi::Element rightEle(hit.ptr->elementSymbol());
		float sigma = selfEle.vdw_r() + rightEle.vdw_r();

		if (dist > sigma - 0.1)
		{
			return;
		}

		if (!_probe || !rightExist)
		{
			return;
		}

		// if both atoms' existence is already certain, this term is a
		// constant for every remaining search outcome - not worth an
		// energy wrapper at all, same reasoning ExhaustiveSearch's own
		// _wider filter already applies (see its constructor).
		if (_existence->is_certain() && rightExist->is_certain())
		{
			return;
		}

		float epsilon = sqrtf(vdw_epsilon_for_element(selfEle) *
		                      vdw_epsilon_for_element(rightEle));

		// staged, not built into an EnergyWrapper yet - Network::
		// bundleRepulsionTerms() runs once after every atom's
		// clashLogic() has had a turn, groups atoms by
		// mutualExistenceNeighbours() connectivity, and builds exactly
		// one shared wrapper per group from whichever member's candidate
		// is closest, instead of one wrapper per atom.
		_probe->setPendingRepulsion({_existence, rightExist, dist, sigma, epsilon});
	};

	ExistenceConnector *&left = _existence;
	for (const AtomConf &hit : hits)
	{
		ExistenceConnector *right = existMap()[hit];

		if (hit.ptr->elementSymbol() == "NA") // not a proper handling of metals
		{
			continue;
		}

		// we do not care about two symmetry-related atoms
		if (hit.ptr->symmetryCopyOf() && _atomConf.ptr->symmetryCopyOf())
		{
			continue;
		}

		float l = glm::length(_atomConf.position() - hit.position());

		bool involvesHydrogen = (_atomConf.ptr->elementSymbol() == "H" ||
		                          hit.ptr->elementSymbol() == "H");

		// same boundaries the hard clash logic has always used, just
		// named now so the gap beyond it (out to the pair's own van der
		// Waals radius sum - see add_repulsion) can get a softer
		// repulsion term instead of being ignored outright.
		float strict_cutoff = involvesHydrogen ? 1.5f : 2.0f;

		// assume freely rotatable hydrogens will find a way not to clash
		if (is_twirling_hydrogen(hit.ptr) || is_twirling_hydrogen(_atomConf.ptr))
		{
			strict_cutoff = 1.0f;
		}

		if (l > strict_cutoff)
		{
			bool becameClosest = (l < closestDist);

			if (becameClosest)
			{
				closestDist = l;
				closestCandidate = hit;
				closestRight = right;
				haveCandidate = true;
			}
			continue;
		}

		// problem - could be deprotonated...
		Existence::Values left_before = left->value();
		Existence::Values right_before = right->value();

		std::cout << "Organising a clash between " << *left << " (element "
		<< _atomConf.ptr->elementSymbol() << ") and "
		<< *right << " (element " << hit.ptr->elementSymbol() << 
		") due to length " << l << std::endl;

		std::ostringstream result;
		try
		{
			add_constraint(new MaxOne(*left, *right));

			// registered into the same others()/register_probe() graph
			// Network::establishAtom() already uses for alt-conf siblings
			// - Subdivide::finish_ends()'s add_alt_confs block walks any
			// direct atom<->atom edge regardless of what it represents,
			// so a clashing pair now gets pulled into the same subdivision
			// chunk together the same way alt-conf siblings already are.
			// Never mistaken for a real bonded neighbour by the GUI's
			// Probe::bondedNeighbours() (see its own comment), since that
			// only ever follows edges reached by crossing an actual bond
			// probe, never a direct registration like this one.
			//
			// probeForAtom() returns nullptr for anything not tracked as
			// an AtomProbe - notably a hydrogen causing the clash, which
			// lives in the separate probeForHydrogen() map instead (also
			// possible: an atom findNeighbours() reached before its own
			// establishAtom() has run yet, mid-construction, still
			// nullptr either way) - clashLogic() runs during Network's
			// own constructor, so this is a real, not just theoretical,
			// case. Fall back to probeForHydrogen() rather than silently
			// dropping the edge, since a hydrogen clash needs following
			// too; only skip registering entirely if neither map has it.
			Probe *rightProbe = _network.probeForAtom(hit);
			if (!rightProbe)
			{
				rightProbe = _network.probeForHydrogen(hit);
			}

			if (rightProbe)
			{
				_probe->register_probe(rightProbe);
				rightProbe->register_probe(_probe);
			}
		}
		catch (const std::runtime_error &err)
		{
			result << "Failed to add constraint between " << *left
			<< " and " << *right << " as it led to immediate contradiction";
			std::cout << result.str() << std::endl;

			_network.addImpromptuCollapse(result.str());

			continue;
		}

		result << "Clash between " << *left << " and "
		<< *right << " prematurely collapsed alternate conformer of ";

		Existence::Values left_after = left->value();
		Existence::Values right_after = right->value();
		
		bool lChange = left_before != left_after;
		bool rChange = right_before != right_after;
		
		if (lChange)
		{
			result << "the former";
		}
		if (lChange && rChange)
		{
			result << " and ";
		}
		if (rChange)
		{
			result << "the latter";
			if (is_placeholder_hydrogen_name(hit.ptr->atomName()))
			{
				continue;
			}
		}
		
		if (lChange || rChange)
		{
			_network.addImpromptuCollapse(result.str());
		}
	}

	if (haveCandidate)
	{
		add_repulsion(closestCandidate, closestDist, closestRight);
	}
}

