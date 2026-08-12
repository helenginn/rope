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

#ifndef __vagabond__Probe__
#define __vagabond__Probe__

#define Z_DEF (-0)
#include "Connector.h"
#include "Guilt.h"
#include <memory>
#include <optional>
#include <string>
#include <vagabond/core/Atom.h>
#include <vagabond/core/Responder.h>

namespace hnet
{
	typedef std::function<float()> GetEnergy;

	// GuiltVersion here is a round identifier, not a blame tag as
	// elsewhere - a wrapper that wants to be counted at most once per
	// scoring round (e.g. a repulsion term shared across alt-conf
	// siblings) can compare it against a memoised "last round I fired
	// in" to tell a fresh round apart from a repeat call within the same
	// one. Every other wrapper is free to ignore it.
	typedef std::function<GetEnergy(GuiltVersion)> EnergyWrapper;
};

class Probe : public HasResponder<Responder<Probe>>
{
public:
	virtual const glm::vec3 &position() const
	{
		return _pos;
	}
	
	virtual ~Probe()
	{

	}
	
	const glm::vec3 colour() const
	{
		return _colour;
	}
	
	const glm::vec4 glow() const
	{
		return _glow;
	}
	
	void setBulk(const bool &bulk)
	{
		_bulk = bulk;
	}

	void setColour(const glm::vec3 &colour)
	{
		_colour = colour;
	}

	void setPosition(const glm::vec3 &position)
	{
		_pos = position;
	}

	virtual bool is_absent()
	{
		return false;
	}

	virtual bool is_covalent()
	{
		return false;
	}

	virtual bool is_bond()
	{
		return false;
	}

	virtual bool is_bulk()
	{
		return _bulk;
	}

	virtual bool is_definitely_not_present()
	{
		return false;
	}

	/** true unless this is an AtomProbe whose chemistry connector has
	 * resolved to hnet::Atom::Inactive (i.e. not N/O/S/ion - carbon and
	 * anything else that can't be an H-bond donor/acceptor). The single
	 * source of truth for "is this atom H-bond-relevant", set once in
	 * Coordinated::probeAtom() - callers should prefer this over
	 * re-deriving the same thing from Atom::elementSymbol() themselves.
	 * Default true: only AtomProbe restricts it, so this is safe to call
	 * on any Probe without an is_atom() guard first. Says nothing about
	 * hydrogens (never AtomProbe, so never Inactive here) or covalent
	 * bonds (a separate axis - see is_covalent()). */
	virtual bool isActiveAtom()
	{
		return true;
	}

	virtual std::string desc() = 0;

	void register_probe(Probe *other)
	{
		_others.push_back(other);
	}

	std::vector<Probe *> &others()
	{
		return _others;
	}

	/** side-channel only - deliberately does NOT also call register_probe()
	 * / touch others(), unlike a plain tag would. Network::
	 * setupInactiveAtom()'s make_certain_covalent_bond() is the one
	 * caller: when a covalent bond's matching-letter conformer resolves
	 * unambiguously AND occupancies agree (diff < 0.05), it uses
	 * hnet::MutualExistence rather than SubExistence, meaning both atoms'
	 * existence is tied together - unlike a bond-probe-mediated edge
	 * (already in others(), reached one hop further out through the
	 * BondProbe), this is a genuinely new direct atom<->atom link that
	 * would otherwise perturb hop-based traversal (Subdivide/
	 * CliqueFinder/ExhaustiveSearch's _wider expansion) by one hop for no
	 * reason relevant to any of them - so it stays out of others()
	 * entirely, visible only via mutualExistenceNeighbours(), for the one
	 * thing that needs it: bundling a shared clash-repulsion term
	 * (Coordinated::add_repulsion) so a scoring round only counts it
	 * once. Unlike alt-conf siblings (mutually EXCLUSIVE existence, so
	 * their own repulsion wrappers already can't double-count each
	 * other), several mutual-existence neighbours really can be
	 * simultaneously Present and each independently contribute a full
	 * closure to every ProbeResult unless bundled. */
	void registerMutualExistence(Probe *other)
	{
		_mutualExistenceNeighbours.push_back(other);
	}

	/** atoms whose existence is unambiguously tied to this one - see
	 * registerMutualExistence()'s own comment for exactly which bonds
	 * qualify. First-order only: this is not transitive (a neighbour's
	 * own further neighbours are not included). */
	std::vector<Probe *> &mutualExistenceNeighbours()
	{
		return _mutualExistenceNeighbours;
	}

	/** this atom's own closest clash-repulsion candidate, staged by
	 * Coordinated::clashLogic() (Coordinated::add_repulsion) rather than
	 * built into an EnergyWrapper immediately - Network::
	 * bundleRepulsionTerms() runs once after every atom's clashLogic()
	 * has had a turn, groups atoms by mutualExistenceNeighbours()
	 * connectivity, and builds exactly one shared wrapper per group from
	 * whichever member's candidate is closest, rather than one wrapper
	 * per atom. selfExist/targetExist are captured here (rather than
	 * re-derived from a Probe* later) since is_atom()/is_bond() Probe
	 * subtypes don't share a common virtual existence() accessor. */
	struct PendingRepulsion
	{
		hnet::ExistenceConnector *selfExist = nullptr;
		hnet::ExistenceConnector *targetExist = nullptr;
		float dist = 0;
		float sigma = 0;
		float epsilon = 0;
	};

	void setPendingRepulsion(const PendingRepulsion &pending)
	{
		_pendingRepulsion = pending;
	}

	std::optional<PendingRepulsion> &pendingRepulsion()
	{
		return _pendingRepulsion;
	}

	/** collects every Probe reached from a BondProbe's own others() (which
	 *  only ever holds its endpoints - see the BondProbe constructor)
	 *  that is not itself another bond - i.e. its endpoint atom(s). Used
	 *  only by bondedNeighbours() below, to make sure a neighbour is only
	 *  ever counted as "bonded" once actually reached via crossing a real
	 *  bond probe, never via a direct (non-bond-mediated) registration on
	 *  the atom itself. */
	static void bondEndpoints(Probe *bondProbe, OpSet<Probe *> &growing)
	{
		for (Probe *other : bondProbe->others())
		{
			if (!other->is_bond())
			{
				growing.insert(other);
			}
		}
	}

	/** one hop of "bonded" reach from `base` - covalent bonds AND
	 *  hydrogen-bond halves both count (both are is_bond()), since you
	 *  want H-bonded atoms pulled close on screen too (a bond line gets
	 *  drawn between them) - but, critically, ONLY atoms reached by
	 *  actually crossing one of those bond probes. others() also carries
	 *  several kinds of direct (non-bond-mediated) atom<->atom edges -
	 *  alt-conformer siblings, steric clashes, charge-sharing/tautomer
	 *  partners, bulk-liberated-water pseudo-conformers, crystallographic
	 *  symmetry mates (see Network::establishAtom/shareProperty,
	 *  Coordinated::clashLogic, ProtonNetworkView::linkSymmetricAtomProbes)
	 *  - none of which are a real bonded neighbour, and none of which
	 *  should make two Probes spring together in a 2D layout (that
	 *  coupling was the actual bug this method exists to avoid - see
	 *  Subdivide.cpp for the very different, and correct, place those
	 *  other kinds of edges DO matter). Shared by ProtonNetworkView::
	 *  arrangeFigure() and HBondDiagram::makeAtoms(), which both build a
	 *  "1 and 2 bonds away" reach set from this (called twice, once per
	 *  hop, snapshotting the 1-hop result first so 2-hop atoms are not
	 *  themselves re-walked as if they were 1-hop) to weight
	 *  PositionShifter's 2D-layout springs - they differ only in which of
	 *  the returned Probes they actually keep (already displayed, not
	 *  hidden, etc.), so this deliberately returns everything reachable
	 *  and leaves that filtering to the caller rather than taking a
	 *  membership test itself. */
	static void bondedNeighbours(Probe *base, OpSet<Probe *> &growing)
	{
		for (Probe *other : base->others())
		{
			if (other->is_bond())
			{
				bondEndpoints(other, growing);
			}
		}
	}
	
	virtual bool is_text() = 0;
	virtual std::string display() = 0;
	virtual std::string value()
	{
		return "";
	}

	virtual bool is_atom()
	{
		return false;
	}
	
	virtual bool is_certain() = 0;

	// the probe's current resolved value as a plain int, for recording
	// into a ProbeResult/OneProbe. Meaningful once is_certain() is true;
	// callers should check that first. `type` matters for BondProbe,
	// which is tracked as two separate ProbeTypePair slots (its own
	// existence, and - once present - which kind of bond); other probe
	// types only ever have one meaningful value and ignore it.
	virtual int certainValueAsInt() = 0;
	
	void setHide(float alpha, OpSet<Probe *> &fixed)
	{
		_hide = alpha;
		fixed.insert(this);
		
		for (Probe *other : _others)
		{
			if (fixed.count(other) == 0)
			{
				other->setHide(_hide, fixed);
			}
		}
		sendResponse("alpha", this);
	}

	void setHide(float alpha, bool recursive = true)
	{
		if (recursive)
		{
			OpSet<Probe *> fixed;
			setHide(alpha, fixed);
		}
		else
		{
			_hide = alpha;
			sendResponse("alpha", this);
		}
	}

	virtual float transparency()
	{
		return 0.f;
	}

	float alpha()
	{
		return std::max(-1.f, _hide + transparency());
	}

	void setMult(const float &m)
	{
		_mult = m;
	}
	
	float mult()
	{
		return _mult;
	}

	// appends rather than replaces - a probe can pick up more than one
	// independent energy contribution (e.g. an atom clashing with
	// several neighbours each gets its own repulsion wrapper added via
	// Coordinated::clashLogic()), all summed together by energy() below.
	void addEnergyWrapper(const hnet::EnergyWrapper &f)
	{
		if (f)
		{
			_energy.push_back(f);
		}
	}

	hnet::GetEnergy energy(GuiltVersion gv)
	{
		std::vector<hnet::GetEnergy> jobs;
		for (const hnet::EnergyWrapper &wrapper : _energy)
		{
			hnet::GetEnergy job = wrapper(gv);
			if (job)
			{
				jobs.push_back(job);
			}
		}

		if (jobs.size() == 0)
		{
			return {};
		}

		return [jobs]()
		{
			float total = 0;
			for (const hnet::GetEnergy &job : jobs)
			{
				total += job();
			}
			return total;
		};
	}

	void realign()
	{
		if (_realign) { _realign(); };
	}
	
	::Atom *const &atom() const
	{
		return _atom;
	}
	
	virtual hnet::AtomConf atomConf() const
	{
		return {_atom, _conf};
	}
	
	Atom *_atom = nullptr;
	Atom *_h = nullptr;
	char _conf;
	glm::vec3 _pos = {};
	glm::vec3 _init = {};
	glm::vec4 _glow = {};
	
	std::vector<Probe *> _others;
	std::vector<Probe *> _mutualExistenceNeighbours;
	std::optional<PendingRepulsion> _pendingRepulsion;
	glm::vec3 _colour = {};
	float _mult = 25;
	float _hide = 0.f;
	bool _bulk = false;
	std::vector<hnet::EnergyWrapper> _energy;

	std::function<void()> _realign{};
};

class AtomProbe : public Probe
{
public:
	AtomProbe(hnet::AtomConnector &obj, hnet::ExistenceConnector &exist,
	          Atom *inherit = nullptr, char conf = '\0', 
	          const std::string &custom_text = {})
	: _obj(obj), _exist(exist)
	{
		if (inherit)
		{
			_atom = inherit;
			_conf = conf;
			hnet::AtomConf ac = {_atom, _conf};
			_init = ac.position();
			_colour = glm::vec3(-0.3f);
			if (_atom->code() == "HOH")
			{
				_colour = glm::vec3(0.6, -0.2, -0.2);
				_glow = {0.3f, 0.3f, 2.f, 1.f};
			}
		}

		_pos = _init;
		_text = custom_text;
	}

	virtual bool is_text()
	{
		return true;
	}
	
	virtual bool is_atom()
	{
		return true;
	}

	virtual bool isActiveAtom()
	{
		return _obj.value(true) != hnet::Atom::Inactive;
	}

	virtual std::string desc()
	{
		return atomConf().desc();
	}

	virtual std::string display()
	{
		if (_text.length())
		{
			return _text;
		}

		std::string str;
		hnet::Atom::Values val = _obj.value(true);

		switch (val)
		{
			case hnet::Atom::Oxygen:
			str = "O";
			break;

			case hnet::Atom::Nitrogen:
			str = "N";
			break;

			case hnet::Atom::Sulphur:
			str = "S";
			break;

			case hnet::Atom::Inactive:
			str = " ";
			break;

			case hnet::Atom::Ion:
			str = "M";
			break;

			case hnet::Atom::Contradiction:
			str = "!";
			break;

			default:
			str = "?";
			break;
		}
		
		if (str == "M" && _atom)
		{
			str = _atom->elementSymbol();
			if (str.length() > 1)
			{
				str[1] = std::tolower(str[1]);
			}
		}
		
		return str;
	}

	virtual float transparency()
	{
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.value(true) == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}
	
	virtual bool is_certain()
	{
		return _obj.is_certain() && _exist.is_certain();
	}

	virtual int certainValueAsInt()
	{
		return (int)_exist.value();
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	std::string _text;
	hnet::AtomConnector &_obj;
	hnet::ExistenceConnector &_exist;
};

class HydrogenProbe : public Probe
{
public:
	HydrogenProbe(hnet::ExistenceConnector &obj, 
	              hnet::ExistenceConnector &exist, Atom *hAtom,
	              AtomProbe *left = {}, AtomProbe *right = {}) :
	_obj(obj), _exist(exist), _left(left), _right(right)
	{
		_init = hAtom->initialPosition();
		_pos = _init;
		_h = hAtom;
		_conf = hAtom->conformerPositions().begin()->first[0];

		_colour = glm::vec3(0.28f, 0.1f, -0.147f);
	}
	
	virtual hnet::AtomConf atomConf() const
	{
		return {_h, _conf};
	}
	
	virtual std::string display()
	{
		if (!_right) return "";
		std::string str;

		// _exist (hExist) is a separate connector from _obj (h, the
		// protonation state/role) - h alone can settle to Present even
		// while hExist independently settles to Absent (e.g. Coordinated_
		// Hydrogens.cpp's StrictExistence forcing hExist Absent on a
		// broken bond, via a constraint that says nothing at all about h),
		// so h can't be trusted alone to mean "this hydrogen is really
		// here" - same reasoning as certainValueAsInt() below.
		if (_exist.value(true) == hnet::Existence::Absent)
		{
			return " ";
		}

		hnet::Existence::Values val = _obj.value(true);

		switch (val)
		{
			case hnet::Existence::Absent:
			str = " ";
			break;

			case hnet::Existence::Present:
			str = "H";
			break;

			case hnet::Existence::Contradiction:
			str = "!";
			break;

			case hnet::Existence::Unassigned:
			str = "?";
			break;

			default:
			str = "?";
			break;
		}

		return str;
	}

	void setAtomPosition(const glm::vec3 &pos)
	{
		_h->setInitialPosition(pos);
	}
	
	void setPosition(const glm::vec3 &pos)
	{
		_pos = pos;
		_h->setInitialPosition(pos);
		_obj._update(true);
	}

	virtual bool is_text()
	{
		return true;
	}

	virtual std::string desc()
	{
		return _obj.desc();
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	virtual bool is_definitely_not_present()
	{
		// same reasoning as certainValueAsInt()/display() - _obj (h)
		// alone doesn't mean this hydrogen is really here; _exist
		// (hExist) can independently be Absent regardless of what h says.
		return (_obj.value() == hnet::Existence::Absent ||
		        _exist.value() == hnet::Existence::Absent);
	}

	virtual float transparency()
	{
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.is_certain(true) &&
		         _exist.value(true) == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}


	virtual bool is_certain()
	{
		return _obj.is_certain() && _exist.is_certain();
	}

	virtual int certainValueAsInt()
	{
		// same collapse BondProbe::certainValueAsInt() already does for
		// its own _exist/_obj pair - h (_obj) alone is not a reliable
		// signal of whether this hydrogen is really present, since
		// hExist (_exist) can independently settle Absent regardless of
		// what h itself resolves to (see display()'s own comment) - only
		// called once is_certain() is true, so both are already known
		// settled by this point, just not necessarily consistent with
		// each other.
		if (_exist.value() == hnet::Existence::Absent)
		{
			return (int)hnet::Existence::Absent;
		}

		return (int)_obj.value();
	}

	virtual bool is_absent()
	{
		// same reasoning as certainValueAsInt()/display() above.
		return (_obj.value() == hnet::Existence::Absent ||
		        _exist.value() == hnet::Existence::Absent);
	}

	hnet::ExistenceConnector &_obj;
	hnet::ExistenceConnector &_exist;
	AtomProbe *_left{};
	AtomProbe *_right{};
};


class BondProbe : public Probe
{
public:
	BondProbe(hnet::BondConnector &obj, Probe &left, Probe &right,
          hnet::ExistenceConnector &exist) :
	_obj(obj), _left(left), _right(right), _exist(exist)
	{
		_init = left.position();
		_pos = _init;
		
		left.register_probe(this);
		right.register_probe(this);
		
		register_probe(&left);
		register_probe(&right);
	}

	virtual const glm::vec3 &position() const
	{
		return _left.position();
	}
	
	const glm::vec3 &end() const
	{
		return _right.position();
	}

	virtual bool is_text()
	{
		return false;
	}

	Probe &left() const
	{
		return _left;
	}

	Probe &right() const
	{
		return _right;
	}

	virtual std::string desc()
	{
		return _obj.desc();
	}

	virtual bool is_definitely_not_present()
	{
		return !(_obj.value() & hnet::Bond::Bonded);
	}

	virtual std::string value()
	{
		std::ostringstream ss;
		ss << "{" << _obj.value() << "} and ";
		ss << "{" << _exist.value() << "}";
		return ss.str();
	}

	virtual float transparency()
	{
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.is_certain(true) &&
		         _exist.value(true) == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}


	virtual bool is_bond()
	{
		return true;
	}

	virtual std::string display()
	{
		std::string str;
		hnet::Bond::Values val = _obj.value(true);

		switch (val)
		{
			case hnet::Bond::Weak:
			str = "weak_bond";
			break;

			case hnet::Bond::Strong:
			str = "strong_bond";
			break;

			case hnet::Bond::LonePair:
			str = "lone_pair";
			break;

			case hnet::Bond::Broken:
			str = "transparency";
			break;

			case hnet::Bond::NotBonded:
			str = "transparency";
			break;

			case hnet::Bond::Bonded:
			str = "present_bond";
			break;

			default:
			str = "unassigned_bond";
			break;
		}
		
		return str;
	}

	virtual bool is_certain()
	{
		if (_exist.value() == hnet::Existence::Absent)
		{
			return true;
		}

		return _obj.is_certain() && _exist.is_certain();
	}

	virtual int certainValueAsInt()
	{
		// same collapse for both slots: absent existence or a
		// functionally-absent bond state (broken/lone pair) means there
		// is nothing more to report either way.
		if (_exist.value() == hnet::Existence::Absent)
		{
			return (int)hnet::Existence::Absent;
		}
		if (_obj.value() == hnet::Bond::Broken ||
		    _obj.value() == hnet::Bond::LonePair)
		{
			return (int)hnet::Existence::Absent;
		}

		return (int)_obj.value();
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	hnet::BondConnector &_obj;
	Probe &_left;
	Probe &_right;
	hnet::ExistenceConnector &_exist;
	float _distance{};
};


#endif
