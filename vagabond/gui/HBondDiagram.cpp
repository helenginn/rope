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

#include "HBondDiagram.h"

#include <vagabond/core/PositionShifter.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/gui/BondRod.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/GLView.h>
#include <vagabond/gui/elements/Window.h>

HBondDiagram::HBondDiagram(const OpSet<ProbeTypePair> &nodes,
                           const glm::mat4x4 &model)
{
	_nodes = nodes;
	_shifter = new PositionShifter(model);

	// same scheme as ProtonNetworkView::arrangeFigure() - a real distance-
	// matching spring (the default targetFn, Probe::_init Euclidean
	// distance - see PositionShifter::gradient()) only makes sense between
	// atoms actually related through the covalent bond graph, not every
	// pair; weight 0 makes the spring term a no-op for everything else.
	_shifter->setWeightFn([this](void *left, void *right) -> float
	{
		auto found = _reach.find(left);
		if (found == _reach.end())
		{
			return 0.f;
		}
		return found->second.count(right) ? 1.f : 0.f;
	});

	// binary weightFn (1.0/0.0) - exempt already-spring-held bonded pairs
	// from also getting a repulsion kick, matching ProtonNetworkView's
	// own arrangeFigure() (see setExemptWeightedFromRepulsion()'s comment
	// for why - otherwise a NaN-poisoned, permanently frozen atom is a
	// real risk here too).
	_shifter->setExemptWeightedFromRepulsion(true);

	makeAtoms();
	makeBonds();

	// without this, the screen only actually redraws on unrelated input
	// (e.g. mouse movement) - _gl is null until this object's first
	// render() call sets it (Renderable::render()), so this Tidy job is a
	// no-op for whatever few cycles land before that.
	_shifter->addTidy([this]()
	{
		if (_gl)
		{
			_gl->viewChanged();
		}
	});
}

HBondDiagram::~HBondDiagram()
{
	// stops and joins the physics thread before Box's own destructor gets
	// to delete the label/bond Renderables its Tidy jobs and getters/
	// setters still hold raw pointers to.
	delete _shifter;
	_shifter = nullptr;
}

void HBondDiagram::start()
{
	_shifter->run();
}

void HBondDiagram::makeAtoms()
{
	std::vector<Probe *> atomProbes;
	OpSet<Probe *> seen;

	for (const ProbeTypePair &ptp : _nodes)
	{
		Probe *probe = ptp.first;
		if (probe && probe->is_atom() && seen.count(probe) == 0)
		{
			atomProbes.push_back(probe);
			seen.insert(probe);
		}
	}

	// also pull in every bond's two endpoints, even ones not otherwise
	// present as their own ExistenceType ptps() entry - a half-H-bond's
	// two Probe endpoints are one heavy atom and the bridging hydrogen
	// itself (see Coordinated.cpp, the BondProbe(left, *ref, hProbe, le)
	// construction), and ExhaustiveSearch deliberately never records a
	// raw hydrogen atom as its own ExistenceType entry (elementSymbol()
	// == "H" is skipped outright) - so without this, every H-bond would
	// be missing its hydrogen endpoint and makeBonds() would never find
	// both ends present, and draw nothing at all.
	for (const ProbeTypePair &ptp : _nodes)
	{
		Probe *probe = ptp.first;
		if (!probe || !probe->is_bond())
		{
			continue;
		}

		BondProbe *bp = static_cast<BondProbe *>(probe);
		for (Probe *end : {&bp->left(), &bp->right()})
		{
			if (seen.count(end) == 0)
			{
				atomProbes.push_back(end);
				seen.insert(end);
			}
		}
	}

	if (atomProbes.empty())
	{
		return;
	}

	glm::vec3 centroid{};
	for (Probe *probe : atomProbes)
	{
		centroid += probe->_init;
	}
	centroid /= (float)atomProbes.size();

	// this Scene's shaders (see box.vsh) treat vertex positions as
	// literal final NDC coordinates, with no camera/projection to bring
	// Probe::_init's real, Angstrom-scale coordinates back into [-1,1] -
	// unlike ProtonNetworkView, a genuine 3D camera scene where
	// arrangeFigure() can use _init directly. Picks a scale so the atom
	// furthest from the group's own centroid lands at about half a
	// screen unit - see _displayScale's own comment for why the shared
	// centroid subtraction doesn't need to match on both sides of the
	// spring-target comparison, only this scale factor does.
	float maxDist = 0.01f;
	for (Probe *probe : atomProbes)
	{
		float d = glm::length(probe->_init - centroid);
		if (d > maxDist)
		{
			maxDist = d;
		}
	}
	_displayScale = 0.5f / maxDist;

	// NDC x and y units do not span equal physical screen distances
	// unless the window happens to be square - PositionShifter's physics
	// (repulsion/springs) is isotropic in "sim space" and has no notion
	// of screen shape at all, so without this the whole layout rendered
	// visibly stretched/squashed on any non-square window, not just the
	// bond widths (see flat_bond.vsh for that separate correction - a
	// bond's own perpendicular width direction needs correcting too,
	// even once the positions it connects are already right). Applied
	// only when converting to/from actual screen (rendered) coordinates,
	// never inside the physics loop itself - get_pos/set_pos are the
	// only boundary between the two.
	float aspect = Window::aspect();
	auto to_screen = [aspect](glm::vec3 v) { v.x *= aspect; return v; };
	auto to_sim = [aspect](glm::vec3 v) { v.x /= aspect; return v; };

	for (Probe *probe : atomProbes)
	{
		Text *label = new Text(probe->display());
		label->resize(0.5);
		label->setPosition(to_screen((probe->_init - centroid) * _displayScale));
		// matches ProbeAtom::fullUpdateProbe()'s own setAlpha(_probe->
		// alpha()) - fades an atom that is not certain, and further fades
		// (to fully invisible) one that is certainly absent, same as the
		// interactive 3D view. A one-shot snapshot here, not a live
		// update callback, same as the display() text itself.
		label->setAlpha(probe->alpha());
		addObject(label);

		_atoms[probe] = label;

		auto get_init = [probe]() -> glm::vec3 { return probe->_init; };
		auto get_pos = [label, this, to_sim]() -> glm::vec3
		{
			return to_sim(label->centroid()) / _displayScale;
		};
		auto set_pos = [label, this, to_screen](const glm::vec3 &pos)
		{
			label->setPosition(to_screen(pos * _displayScale));
		};

		_shifter->addPosition(probe, get_init, get_pos, set_pos);
		_shifter->addTidy([label]() { label->forceRender(true, false); });
	}

	// per-atom 1-and-2-hop covalent-bond reach, via the graph walk shared
	// with ProtonNetworkView::arrangeFigure() (Probe::bondedNeighbours()
	// - see its own comment) - restricted here to Probes actually in
	// _atoms, since anything outside this diagram's own node set has no
	// Renderable to weight a spring against anyway. Deferred until _atoms
	// is fully populated - membership has to be complete before computing
	// reach for any atom, or atoms processed earlier could miss ones
	// added later.
	for (auto &entry : _atoms)
	{
		Probe *probe = entry.first;

		OpSet<Probe *> neighbours;
		Probe::bondedNeighbours(probe, neighbours);

		// 2 bonds away - a snapshot of the 1-hop set first, since
		// extending `neighbours` while iterating it directly would let
		// newly-inserted 2-hop atoms also get walked as if they were
		// 1-hop, silently reaching 3+ bonds out.
		OpSet<Probe *> hopOne = neighbours;
		for (Probe *other : hopOne)
		{
			Probe::bondedNeighbours(other, neighbours);
		}

		OpSet<void *> reach;
		for (Probe *other : neighbours)
		{
			if (_atoms.count(other))
			{
				reach.insert(other);
			}
		}

		_reach[probe] = reach;
	}
}

void HBondDiagram::makeBondLine(Probe *probe)
{
	BondProbe *bp = static_cast<BondProbe *>(probe);
	Probe *left = &bp->left();
	Probe *right = &bp->right();

	if (_atoms.count(left) == 0 || _atoms.count(right) == 0)
	{
		// one or both endpoints aren't among this diagram's own
		// nodes - nothing on screen to connect it to.
		return;
	}

	BondRod *bond = new BondRod("assets/images/" +
	                           probe->display() + ".png");

	// BondRod's default shaders (axes.vsh/.fsh) apply the Scene's
	// model/projection matrices - correct for ProbeBond's real 3D
	// camera scene (ProtonNetworkView), but wrong here: this Scene's
	// Text/Image labels (box.vsh) ignore model/projection entirely
	// and treat vertex positions as already being final NDC
	// coordinates, while Probe::_init (this diagram's positions,
	// scaled by _displayScale but otherwise untouched) still carries
	// a real 3D Z component - fed through a perspective matrix
	// calibrated for an entirely different scene, that produced
	// wildly wrong on-screen positions (bonds effectively invisible,
	// nowhere near the atom labels they should connect). flat_bond.vsh
	// keeps axes.vsh's width-expansion trick but skips the model/
	// projection multiply, matching box.vsh's convention instead - and
	// (unlike axes.vsh, whose width is a fixed fraction of the bond's
	// own length - fine at ProtonNetworkView's real, un-shrunk scale)
	// uses a fixed absolute width instead, since _displayScale can
	// otherwise shrink bonds down to a barely-visible sliver.
	bond->setVertexShaderFile("assets/shaders/flat_bond.vsh");
	bond->setUsesProjection(false);
	// matches ProbeBond::updateProbe()'s own setAlpha(_probe->alpha()) -
	// order-independent with fixVertices() below (setAlpha only ever
	// touches vertex colour, fixVertices only ever touches pos/normal/
	// tex), so this is fine to set once here rather than every tick.
	bond->setAlpha(probe->alpha());
	addObject(bond);

	Text *leftLabel = _atoms[left];
	Text *rightLabel = _atoms[right];

	// truncate a quarter of the length in from an end ONLY where that
	// end actually has a visible label to stop short of - matches
	// ProbeBond::updatePosition()'s own left/right multiplier logic
	// (only non-zero when that side's display() isn't blank). A "silent"
	// atom - logically inactive, e.g. AtomProbe::display()'s Inactive
	// case for a carbon - displays as a blank space, so there is nothing
	// there to overlap; truncating that end anyway just leaves the rod
	// visibly disconnected from the atom's actual position for no reason.
	// Evaluated once here, not per tick - this diagram is a one-shot
	// snapshot (no live update callbacks the way ProbeBond has), so
	// display() never changes after construction.
	auto blank = [](const std::string &text)
	{
		// ProbeBond::updatePosition() only checks " " (AtomProbe/BondProbe
		// Inactive/Absent), but HydrogenProbe::display() can also return
		// "" outright (no right-hand partner set yet) - both mean no
		// visible glyph there.
		return text.empty() || text == " ";
	};
	bool leftBlank = blank(left->display());
	bool rightBlank = blank(right->display());

	// follows wherever the two endpoint labels are currently rendered
	// (PositionShifter-driven), same idiom as ProbeBond::updatePosition()
	// - not the physics probe position directly.
	_shifter->addTidy([bond, leftLabel, rightLabel, leftBlank, rightBlank]()
	{
		glm::vec3 start = leftLabel->centroid();
		glm::vec3 end = rightLabel->centroid();
		glm::vec3 full = end - start;
		glm::vec3 startTrim = leftBlank ? glm::vec3(0.f) : full * 0.25f;
		glm::vec3 endTrim = rightBlank ? glm::vec3(0.f) : full * 0.25f;
		bond->fixVertices(start + startTrim, full - startTrim - endTrim);
		bond->forceRender(true, true);
	});
}

void HBondDiagram::makeBonds()
{
	OpSet<Probe *> drawn;

	for (const ProbeTypePair &ptp : _nodes)
	{
		Probe *probe = ptp.first;
		if (probe && probe->is_bond())
		{
			makeBondLine(probe);
			drawn.insert(probe);
		}
	}

	// the covalent skeleton connecting these same atoms - _nodes never
	// includes it (see this method's own header comment), so walked
	// directly from each displayed atom's own bond-graph neighbours
	// instead, same traversal Probe::bondedNeighbours() itself uses
	// internally (Probe::others(), one hop). A covalent bond is reachable
	// from both of its own endpoints, so drawn tracks which have already
	// been drawn to avoid doubling up.
	for (auto &entry : _atoms)
	{
		for (Probe *other : entry.first->others())
		{
			if (!other->is_bond() || !other->is_covalent())
			{
				continue;
			}

			// same filter Subdivide.cpp's own bond-graph walk uses
			// (bounded_bfs/finish_ends) - skip a covalent link that is
			// certainly broken/absent (e.g. a severed backbone, an
			// alternate conformer artifact) rather than draw a bond that
			// does not structurally exist for this state.
			if (other->is_definitely_not_present())
			{
				continue;
			}

			if (drawn.count(other))
			{
				continue;
			}

			drawn.insert(other);
			makeBondLine(other);
		}
	}
}
