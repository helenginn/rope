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

#include <mutex>
#include <vagabond/core/PositionShifter.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/gui/BondRod.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/GLView.h>
#include <vagabond/gui/elements/Window.h>

namespace
{
// Box::addAltTag()'s own hover tag positions itself via Renderable::
// setHover()'s _align/realign() call - the fractional (setCentre()/
// setLeft()) layout system every OTHER addAltTag() caller in this
// codebase already went through, but this class's own atom labels never
// do (see makeAtoms()'s own comment: positioned via raw setPosition()
// only) - _align stays at its unset default, so the tag lands wherever
// that default resolves to (screen top-left) instead of near the atom.
// Rather than fight that mismatch, this reports hover/unhover out to a
// caller-supplied callback instead (see HBondDiagram's own constructor
// comment) - HBondDiagram wires it to update one shared, sensibly-
// positioned label instead of relying on Box's own per-object tag at
// all. Also overrides mouseOver()'s return value: Box::mouseOver()
// always returns false regardless of whether it actually found
// something, which (Scene::mouseMoveEvent only swaps in the hand cursor
// when mouseOver() returns true) means no Box/Text-based element in this
// whole codebase ever gets that cursor feedback - returning true here
// fixes that for this class's own atoms specifically.
class HoverAtomLabel : public Text
{
public:
	HoverAtomLabel(const std::string &text, const std::string &desc,
	              std::function<void(const std::string &)> hoverCallback)
	: Text(text), _desc(desc), _hoverCallback(hoverCallback)
	{}

	virtual bool mouseOver()
	{
		if (_hoverCallback)
		{
			_hoverCallback(_desc);
		}

		return true;
	}

	virtual void unMouseOver()
	{
		if (_hoverCallback)
		{
			_hoverCallback("");
		}

		Renderable::unMouseOver();
	}

private:
	std::string _desc;
	std::function<void(const std::string &)> _hoverCallback;
};
}

HBondDiagram::HBondDiagram(const OpSet<ProbeTypePair> &nodes,
                           const glm::mat4x4 &model,
                           const CertainStates *states,
                           const std::vector<float> &stateWeights,
                           HBondDiagram *positionSource,
                           const glm::vec3 &positionOffset,
                           float targetRadius,
                           std::function<void(const std::string &)> hoverCallback)
{
	_nodes = nodes;
	_positionOffset = positionOffset;
	_targetRadius = targetRadius;
	_hoverCallback = hoverCallback;

	if (positionSource)
	{
		_shifter = positionSource->_shifter;
		_ownsShifter = false;
		_positionSource = positionSource;
	}
	else
	{
		_shifter = new PositionShifter(model);
		_ownsShifter = true;

		// same scheme as ProtonNetworkView::arrangeFigure() - a real
		// distance-matching spring (the default targetFn, Probe::_init
		// Euclidean distance - see PositionShifter::gradient()) only
		// makes sense between atoms actually related through the covalent
		// bond graph, not every pair; weight 0 makes the spring term a
		// no-op for everything else. Not set up for a mirror instance
		// (positionSource given) - its own atoms are never registered via
		// addPosition() at all (see makeAtoms()), so nothing would ever
		// call this for them anyway, and configuring it twice would only
		// risk a mirror clobbering the shared shifter's real owner's own
		// weight/exempt setup with an equivalent (same _nodes/topology)
		// but not identical closure.
		_shifter->setWeightFn([this](void *left, void *right) -> float
		{
			auto found = _reach.find(left);
			if (found == _reach.end())
			{
				return 0.f;
			}
			return found->second.count(right) ? 1.f : 0.f;
		});

		// binary weightFn (1.0/0.0) - exempt already-spring-held bonded
		// pairs from also getting a repulsion kick, matching
		// ProtonNetworkView's own arrangeFigure() (see
		// setExemptWeightedFromRepulsion()'s comment for why - otherwise
		// a NaN-poisoned, permanently frozen atom is a real risk here
		// too).
		_shifter->setExemptWeightedFromRepulsion(true);
	}

	computeStateAverages(states, stateWeights);

	if (positionSource)
	{
		// registered before makeAtoms()/makeBonds() below, so this runs
		// before this diagram's own bond-position Tidy jobs (also
		// registered on the same, shared shifter) within the same tick -
		// see syncPositionsFrom()'s own comment. Guarded against both
		// instances' own lifetimes, not just this one's - see _alive's
		// own comment.
		HBondDiagram *source = positionSource;
		std::shared_ptr<std::atomic<bool>> alive = _alive;
		std::shared_ptr<std::atomic<bool>> sourceAlive = positionSource->_alive;

		_shifter->addTidy([this, source, alive, sourceAlive]()
		{
			if (!alive->load() || !sourceAlive->load())
			{
				return;
			}

			syncPositionsFrom(*source);
		});
	}

	makeAtoms();
	makeBonds();

	// without this, the screen only actually redraws on unrelated input
	// (e.g. mouse movement) - _gl is null until this object's first
	// render() call sets it (Renderable::render()), so this Tidy job is a
	// no-op for whatever few cycles land before that.
	std::shared_ptr<std::atomic<bool>> alive = _alive;
	_shifter->addTidy([this, alive]()
	{
		if (!alive->load())
		{
			return;
		}

		if (_gl)
		{
			_gl->viewChanged();
		}
	});
}

HBondDiagram::~HBondDiagram()
{
	// first, before anything else is torn down - see _alive's own
	// comment. The (possibly shared) physics thread may still be mid-tick
	// on a Tidy job referencing this object right up until whichever
	// instance owns it actually stops it, and this is the only signal
	// that reaches across to it in time.
	_alive->store(false);

	// ... but flipping the flag alone only narrows the race, it doesn't
	// close it: a Tidy job that already read _alive as true a moment
	// before the store above can still be mid-execution - e.g. a mirror's
	// syncPositionsFrom(), reading straight off this instance - right up
	// until whatever comes after this destructor's body actually frees
	// the memory it's using. waitForTidy() blocks until any such
	// in-flight call has completely finished before we let that happen -
	// crashed in exactly this window before this existed (a background-
	// thread pthread_mutex_lock deep inside syncPositionsFrom, racing a
	// mirror's destruction from ViewCorrelations::deleteTemps()). Skipped
	// only if _shifter was never set up in the first place.
	if (_shifter)
	{
		_shifter->waitForTidy();
	}

	// stops and joins the physics thread before Box's own destructor gets
	// to delete the label/bond Renderables its Tidy jobs and getters/
	// setters still hold raw pointers to - only if this instance actually
	// owns it (see _shifter's own comment); a mirror instance's own
	// positionSource owns and stops the shared one instead (the
	// waitForTidy() above is what keeps a mirror's own teardown safe,
	// since it has no thread of its own to stop).
	if (_ownsShifter)
	{
		delete _shifter;
	}
	_shifter = nullptr;
}

void HBondDiagram::start()
{
	// a mirror instance (positionSource given to the constructor) is
	// driven entirely by its positionSource's own physics thread - see
	// this class's own constructor comment. Guarded rather than trusting
	// every caller to remember not to call this on one.
	if (!_ownsShifter)
	{
		return;
	}

	_shifter->run();
}

void HBondDiagram::settle(int iterations)
{
	// see this method's own header comment - a mirror has nothing of its
	// own to settle (positionSource's own settle(), not this, is what
	// would actually move anything), and calling PositionShifter::
	// settle() on an already-shared shifter here would run its physics
	// loop redundantly on top of whatever positionSource itself already
	// did.
	if (!_ownsShifter)
	{
		return;
	}

	_shifter->settle(iterations);
}

float HBondDiagram::measuredRadius() const
{
	if (_atoms.empty())
	{
		return 0.f;
	}

	glm::vec3 centroid{};
	for (const auto &entry : _atoms)
	{
		centroid += entry.second->centroid();
	}
	centroid /= (float)_atoms.size();

	float maxDist = 0.f;
	for (const auto &entry : _atoms)
	{
		float d = glm::length(entry.second->centroid() - centroid);
		if (d > maxDist)
		{
			maxDist = d;
		}
	}

	return maxDist;
}

void HBondDiagram::syncPositionsFrom(const HBondDiagram &source)
{
	// same to_screen conversion makeAtoms()'s own set_pos uses - _init/
	// _displayScale are identical between two diagrams built from the
	// same _nodes, so this is safe to recompute independently of source's
	// own copy rather than needing to share it.
	float aspect = Window::aspect();
	auto to_screen = [aspect](glm::vec3 v) { v.x *= aspect; return v; };

	for (auto &entry : _atoms)
	{
		Probe *probe = entry.first;

		auto found = source._atoms.find(probe);
		if (found == source._atoms.end())
		{
			continue;
		}

		// source's own sim-space position already has source's own
		// one-time setPosition() group shift baked in (get_pos() samples
		// the label's own live, already-shifted centroid the moment
		// physics first reads it - see makeAtoms()'s own get_pos), so
		// using it here unmodified would land this diagram's own node
		// exactly on top of source's, not offset the way the two
		// diagrams' own separate setPosition() targets intended -
		// _positionOffset (the constant screen-space delta between the
		// two) corrects for that.
		glm::vec3 simPos = source._shifter->getPosition(probe);
		glm::vec3 pos = to_screen(simPos * _displayScale) + _positionOffset;

		// see _manualOffset's own comment - a no-op for every atom never
		// individually dragged in this diagram specifically. Guarded by
		// the shared shifter's own mutex (this runs on its background
		// thread, as one of its own Tidy jobs), since the mirror's own
		// drag callback below writes to the very same map from the main
		// thread instead.
		{
			std::unique_lock<std::mutex> lock(_shifter->skip_lock());
			auto offset = _manualOffset.find(probe);
			if (offset != _manualOffset.end())
			{
				pos += offset->second;
			}
		}

		entry.second->setPosition(pos);
		entry.second->forceRender(true, false);
	}
}

void HBondDiagram::computeStateAverages(const CertainStates *states,
                                        const std::vector<float> &stateWeights)
{
	if (!states || stateWeights.empty())
	{
		return;
	}

	// ExistenceType pass first (see this method's own header comment) -
	// one entry per atom actually tracked by CertainStates, plus (since
	// ExhaustiveSearch::add_result() started recording it explicitly -
	// see its own comment) the bridging hydrogen's own "protonation
	// state" connector, distinguished from a real heavy atom by
	// !is_atom() (a HydrogenProbe is neither is_atom() nor is_bond()).
	for (const ProbeTypePair &ptp : _nodes)
	{
		if (ptp.second != hnet::Types::ExistenceType || !ptp.first)
		{
			continue;
		}

		float sum = 0.f;
		std::map<int, float> totals = states->proportions(ptp, sum, stateWeights);

		if (sum <= 0.f)
		{
			continue;
		}

		float fracPresent = totals[(int)hnet::Existence::Present] / sum;
		_alphaOverride[ptp.first] = fracPresent - 1.f;

		// the bridging hydrogen's own directly-recorded fraction is
		// authoritative for its own displayed text - preferred over the
		// BondType pass's own bond-inferred fallback further down (see
		// its own comment for why that fallback still exists, for a
		// hydrogen whose own protonation state ptp isn't certain/tracked
		// for whatever reason). A real heavy atom's own display() text is
		// just its element symbol, which never varies with existence
		// certainty, so this must not touch it.
		if (!ptp.first->is_atom())
		{
			_textOverride[ptp.first] = (fracPresent >= 0.5f) ? "H" : " ";
		}
	}

	// BondType pass - see BondProbe::certainValueAsInt() for why only
	// these three raw values ever appear: Existence::Absent (covers a
	// genuinely absent bond as well as a functionally-absent Broken/
	// LonePair one - indistinguishable from each other in this recorded
	// data, so there is no separate layer for them below; a bond that is
	// mostly Absent simply has both the layers below faded down together),
	// Bond::Weak (Acceptor), Bond::Strong (Donor). Rather than picking one
	// "majority" icon, every bond gets both layers overlaid at once (see
	// makeBondLine()), each independently faded by its own fraction of the
	// weighted states, so a genuine mix (e.g. half weak, half strong)
	// reads as a blend rather than being forced into one or the other.
	// a bridging hydrogen (see makeAtoms()'s own comment) is always the
	// shared right()-hand endpoint of exactly two BondProbes - one half-
	// bond to each of the two heavy atoms it sits between (see
	// Coordinated_Constraints.cpp's create_two_half_hydrogen_bonds()) -
	// so the loop below visits every bridging hydrogen twice, once per
	// half. Collected here (Probe* -> the higher of the two halves' own
	// fracBonded) rather than committed straight into _alphaOverride/
	// _textOverride inside the loop, since _nodes (an OpSet) does not
	// guarantee which half is visited first - committing immediately on
	// a first-not-already-set basis meant whichever half happened to be
	// visited first won permanently, silently discarding the other
	// half's own fracBonded even when it was the higher (more
	// confidently bonded) of the two.
	std::map<Probe *, float> hydrogenFracBonded;

	for (const ProbeTypePair &ptp : _nodes)
	{
		if (ptp.second != hnet::Types::BondType || !ptp.first)
		{
			continue;
		}

		float sum = 0.f;
		std::map<int, float> totals = states->proportions(ptp, sum, stateWeights);

		if (sum <= 0.f)
		{
			continue;
		}

		float fracWeak = totals[(int)hnet::Bond::Weak] / sum;
		float fracStrong = totals[(int)hnet::Bond::Strong] / sum;
		float fracBonded = fracWeak + fracStrong;

		Probe *probe = ptp.first;
		_alphaOverride[probe] = fracBonded - 1.f;

		_bondOverlay[probe] = {
			{"weak_bond", fracWeak - 1.f},
			{"strong_bond", fracStrong - 1.f},
		};

		// the heavy-atom endpoint almost always already has its own
		// ExistenceType-derived override from the first pass above; this
		// only ever matters for the rare atom with none (still first-
		// wins, unlike the bridging hydrogen below - a heavy atom is
		// never shared between two BondType ptps the way a bridging
		// hydrogen always is, so there is no equivalent ambiguity to
		// resolve here).
		BondProbe *bp = static_cast<BondProbe *>(probe);
		for (Probe *end : {&bp->left(), &bp->right()})
		{
			if (end->is_atom())
			{
				if (_alphaOverride.count(end) == 0)
				{
					_alphaOverride[end] = fracBonded - 1.f;
				}
			}
			else
			{
				float &best = hydrogenFracBonded[end];
				best = std::max(best, fracBonded);
			}
		}
	}

	// commit the higher of each bridging hydrogen's two halves - "H" once
	// its more-confidently-bonded half is bonded in the majority of the
	// weighted states, blank otherwise, matching HydrogenProbe::display()'s
	// own two options. Only a fallback, for a hydrogen the ExistenceType
	// pass above didn't already cover (its own protonation-state ptp
	// wasn't certain/tracked across the weighted states) - never
	// overwrites a direct value that pass already found, which is the
	// more reliable of the two (see that pass's own comment).
	for (auto &entry : hydrogenFracBonded)
	{
		Probe *hydrogen = entry.first;
		float fracBonded = entry.second;

		if (_alphaOverride.count(hydrogen) == 0)
		{
			_alphaOverride[hydrogen] = fracBonded - 1.f;
		}

		if (_textOverride.count(hydrogen) == 0)
		{
			_textOverride[hydrogen] = (fracBonded >= 0.5f) ? "H" : " ";
		}
	}
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
	_displayScale = _targetRadius / maxDist;

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
		// bridging hydrogens only (see _textOverride's own comment) - "H"
		// once the majority of the weighted states have it actually
		// bonded, blank otherwise, rather than whichever this one-shot
		// snapshot's live display() happens to currently show.
		auto textOverride = _textOverride.find(probe);
		std::string text = textOverride != _textOverride.end() ?
		textOverride->second : probe->display();

		// HoverAtomLabel (see its own comment, above the constructor)
		// reports hover/unhover to _hoverCallback rather than showing its
		// own Box::addAltTag() tag, which would land in the wrong place
		// for a label positioned via raw setPosition() the way every atom
		// here is.
		Text *label = new HoverAtomLabel(text, probe->desc(), _hoverCallback);
		label->resize(0.5);
		label->setPosition(to_screen((probe->_init - centroid) * _displayScale));
		// matches ProbeAtom::fullUpdateProbe()'s own setAlpha(_probe->
		// alpha()) - fades an atom that is not certain, and further fades
		// (to fully invisible) one that is certainly absent, same as the
		// interactive 3D view. A one-shot snapshot here, not a live
		// update callback, same as the display() text itself - unless
		// computeStateAverages() found a state-weighted alpha for this
		// atom instead (see its own comment), which then takes priority.
		auto alphaOverride = _alphaOverride.find(probe);
		label->setAlpha(alphaOverride != _alphaOverride.end() ?
		                alphaOverride->second : probe->alpha());
		addObject(label);

		// HasRenderables::findObject() - the hit-test both hover
		// (mouseOver(), above) and dragging (setDragFunction() further
		// down) depend on - skips anything not isSelectable() before it
		// ever even looks at isDraggable() (see its own "if
		// (!r->isSelectable() ...) continue;"), so without this neither
		// would ever fire no matter what else here is set up. Safe to
		// flip on unconditionally - Renderable::setSelectable() only
		// snapshots the vertices already written above for its own
		// highlight-toggle bookkeeping, no different in effect from a
		// plain flag.
		label->setSelectable(true);

		_atoms[probe] = label;

		// a mirror instance's own atoms are never registered for physics
		// at all - their positions are entirely driven by
		// syncPositionsFrom() instead (see the constructor's own tidy
		// job), so registering them here too would just mean the shared
		// shifter's own physics step tried to spring-relax two
		// independent sets of Renderables toward the same targets.
		if (_ownsShifter)
		{
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

			// manual drag, same idiom as ProtonNetworkView::mouseMoveEvent's
			// own _manual/setSkip() dance (see its own comment there for
			// why plain setPosition(), not the physics-resyncing
			// updatePosition(), is what has to run here): x/y arrive
			// already in this Scene's own final-NDC coordinate space (see
			// Scene::mouseMoveEvent's convertToGLCoords() call before
			// Renderable::drag()), the same space Text::setPosition()
			// already expects everywhere else in this class, so no
			// to_screen() conversion is needed on the way in - only
			// to_sim()/_displayScale on the way to the shifter, matching
			// get_pos()'s own formula, so physics resumes smoothly from
			// wherever the atom was dropped rather than snapping back.
			// undrag() calls this once more with finished=true and x=y=0
			// (not the final position - see Renderable::undrag()), purely
			// to hand the probe back to physics.
			std::shared_ptr<std::atomic<bool>> dragAlive = _alive;
			label->setDragFunction([this, probe, label, to_sim, dragAlive]
			(double x, double y, bool finished)
			{
				if (!dragAlive->load())
				{
					return;
				}

				if (finished)
				{
					_shifter->setSkip(nullptr);
					return;
				}

				glm::vec3 pos((float)x, (float)y, 0.f);
				label->setPosition(pos);
				label->forceRender(true, false);

				glm::vec3 simPos = to_sim(pos) / _displayScale;
				_shifter->setPosition(probe, simPos);
				_shifter->setSkip(probe);
			});
		}
		else
		{
			// mirror instance - there is no physics of its own to drag
			// (see above), but the two diagrams often have different
			// nodes actually legible (each one's own alpha reflects a
			// different state-weighted average - see computeStateAverages())
			// so being able to rearrange this one's own layout
			// independently is still genuinely useful. Rather than moving
			// anything physics-related, this just records a screen-space
			// offset (_manualOffset) that syncPositionsFrom() adds on top
			// of wherever it would otherwise place this atom every tick -
			// so a dragged mirror node keeps tracking its counterpart's
			// own ongoing motion in positionSource, just shifted, instead
			// of freezing in place the moment it's dropped.
			std::shared_ptr<std::atomic<bool>> dragAlive = _alive;
			label->setDragFunction([this, probe, label, to_screen, dragAlive]
			(double x, double y, bool finished)
			{
				if (!dragAlive->load() || finished)
				{
					return;
				}

				glm::vec3 pos((float)x, (float)y, 0.f);
				label->setPosition(pos);
				label->forceRender(true, false);

				// "wherever syncPositionsFrom() would have placed this
				// atom right now, with no manual offset applied" - same
				// formula as that method's own, recomputed here (rather
				// than reused) since it has to run at drag time, not tick
				// time.
				glm::vec3 simPos = _positionSource->_shifter->getPosition(probe);
				glm::vec3 natural = to_screen(simPos * _displayScale) +
				_positionOffset;

				// see syncPositionsFrom()'s own comment - this map is read
				// from the shared shifter's own background thread too.
				std::unique_lock<std::mutex> lock(_shifter->skip_lock());
				_manualOffset[probe] = pos - natural;
			});
		}

		std::shared_ptr<std::atomic<bool>> alive = _alive;
		_shifter->addTidy([label, alive]()
		{
			if (!alive->load())
			{
				return;
			}

			label->forceRender(true, false);
		});
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

	// computeStateAverages() fills _bondOverlay with two independently-
	// faded layers (weak_bond, strong_bond - see its own comment for why
	// not more/fewer) when it has state-weighted data to draw from; a
	// plain live snapshot (states==nullptr, e.g. the covalent skeleton
	// drawn by makeBonds()'s second loop, which never gets an override at
	// all) falls back to exactly one rod at probe->display()/probe->
	// alpha(), same as before this overlay existed.
	auto overlay = _bondOverlay.find(probe);
	std::vector<std::pair<std::string, float>> layers;
	if (overlay != _bondOverlay.end())
	{
		layers = overlay->second;
	}
	else
	{
		auto effectiveAlpha = [this](Probe *p) -> float
		{
			auto found = _alphaOverride.find(p);
			return found != _alphaOverride.end() ? found->second : p->alpha();
		};
		// matches ProbeBond::updateProbe()'s own setAlpha(_probe->alpha()).
		// Plain covalent-skeleton bonds never get an _alphaOverride of
		// their own - ExhaustiveSearch skips is_covalent() probes
		// entirely, so CertainStates never tracks them - and probe->
		// alpha() on a covalent BondProbe only reflects whether that
		// specific link is broken, never whether the atoms it joins are
		// actually present in the states being averaged, which left every
		// skeleton bond fully opaque (including ones into a faded-out
		// atom, or a never-tracked carbon) regardless of the averaged
		// view. Falling back to the mean of the two endpoints' own
		// effective alpha (their own override if they have one, else
		// their own live probe->alpha() - covers carbon endpoints too,
		// which never get an override of their own) fixes that without
		// needing carbons to be tracked at all.
		auto alphaOverride = _alphaOverride.find(probe);
		float alpha = alphaOverride != _alphaOverride.end() ?
		alphaOverride->second :
		0.5f * (effectiveAlpha(left) + effectiveAlpha(right));
		layers.push_back({probe->display(), alpha});
	}

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
	// leftLabel/rightLabel's own text() - not left/right->display() - so
	// this matches whatever _textOverride (see its own comment) actually
	// put on screen for a bridging hydrogen, not this one-shot snapshot's
	// live value.
	bool leftBlank = blank(leftLabel->text());
	bool rightBlank = blank(rightLabel->text());

	// one BondRod per overlay layer, all stacked at the exact same
	// position - alpha-blended on top of each other so a genuine mix
	// (e.g. half weak, half strong) reads as a blend rather than being
	// forced into a single "majority" icon (see computeStateAverages()'s
	// own comment). The plain live-snapshot fallback above always
	// produces exactly one layer, so this reduces to the original
	// single-rod behaviour whenever there is no state-weighted data.
	std::vector<BondRod *> rods;
	for (auto &layer : layers)
	{
		BondRod *bond = new BondRod("assets/images/" + layer.first + ".png");

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
		bond->setAlpha(layer.second);
		addObject(bond);
		rods.push_back(bond);
	}

	// follows wherever the two endpoint labels are currently rendered
	// (PositionShifter-driven), same idiom as ProbeBond::updatePosition()
	// - not the physics probe position directly. One shared job for every
	// layer's rod rather than one job per rod - they always move together.
	std::shared_ptr<std::atomic<bool>> alive = _alive;
	_shifter->addTidy([rods, leftLabel, rightLabel, leftBlank, rightBlank,
	                   alive]()
	{
		if (!alive->load())
		{
			return;
		}

		glm::vec3 start = leftLabel->centroid();
		glm::vec3 end = rightLabel->centroid();
		glm::vec3 full = end - start;
		glm::vec3 startTrim = leftBlank ? glm::vec3(0.f) : full * 0.25f;
		glm::vec3 endTrim = rightBlank ? glm::vec3(0.f) : full * 0.25f;

		for (BondRod *bond : rods)
		{
			bond->fixVertices(start + startTrim, full - startTrim - endTrim);
			bond->forceRender(true, true);
		}
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
