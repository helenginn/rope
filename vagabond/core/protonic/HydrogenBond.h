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

#ifndef __vagabond__HydrogenBond__
#define __vagabond__HydrogenBond__

#include "hnet.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct HydrogenBond : public ConstraintBase
{
	HydrogenBond(BondConnector &left, ExistenceConnector &leftExist,
	             ExistenceConnector &centre, ExistenceConnector &hSlot,
	             BondConnector &right, ExistenceConnector &rightExist)
	: _left(left), _leftExist(leftExist), _protonated(centre), _hSlot(hSlot),
	  _right(right), _rightExist(rightExist)
	{
		prep_constraints_and_forgets(this, {&left, &leftExist, &centre,
		                                     &hSlot, &right, &rightExist});
	}
	
	bool bond_acceptor_or_broken(const Bond::Values &val)
	{
		return (val & Bond::Acceptor) && !(val & Bond::Broken);
	}
	
	bool bond_definitely_present(const Bond::Values &val)
	{
		return (val & Bond::Bonded) && !(val & Bond::NotBonded);
	}
	
	bool bond_definitely_not_bonded(const Bond::Values &val)
	{
		return !(val & Bond::Bonded);
	}
	
	std::string desc()
	{
		return "Hydrogen bonding pattern between \"" + _left.desc() + "\", \"" + 
		_protonated.desc() + "\", \"" + _right.desc() + "\"";
	}
	
	void print_bond()
	{
		std::cout << _left.value() << " " << _protonated.value() << 
		" " << _right.value() << std::endl;
		
		if (_left.value() == Bond::Contradiction)
		{
			_left.report();
		}
		else if (_right.value() == Bond::Contradiction)
		{
			_right.report();
		}
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		// if H is missing, it can only be a lone pair OR broken bond
		if (_protonated.value() == Existence::Absent)
		{
			assign(_left, Bond::NotBonded, "an absent hydrogen cannot be "\
			       "adjacent to a donor or acceptor bond");
		}
		
		// if H is present, one bond must be a donor, other cannot be a donor
		if (_protonated.value() == Existence::Present)
		{
			if (_right.value() == Bond::Donor)
			{
				assign(_left, Bond::NotDonor, "a hydrogen with a donor on"\
				       " one side must be non-donor on the other");
			}
			
			if (!(_right.value() & Bond::Donor))
			{
				assign(_left, Bond::Donor, "a hydrogen with a non-donor on "\
				       "one side must be a donor on the other");
			}

			if (bond_definitely_not_bonded(_right.value()) &&
			    bond_definitely_present(_left.value()))
			{
				assign(_left, Bond::NotAcceptor,
				       "if H-bond is not complete on "\
				       "both sides, remaining side cannot be acceptor");
			}
			// if we only have choice between lone pair and acceptor, it's acceptor
			/* // not the case due to sampling things
			if (_left.value() == Bond::LonePairOrAcceptor)
			{
				assign(_left, Bond::Acceptor, "if we only have choice between lone "\
				       "pair and acceptor, it's acceptor");
			}
			*/
		}

		
		// if we definitely have a donor/acceptor bond then we must have H
		if (bond_definitely_present(_left.value()))
		{
			assign(_protonated, Existence::Present, "a donor/acceptor bond must "\
			       "have a present hydrogen");
		}
		
		if (bond_definitely_not_bonded(_left.value()) && 
		    bond_definitely_not_bonded(_right.value()))
		{
			assign(_protonated, Existence::Absent, "a hydrogen braced by two "\
			       "non-bonds must be absent");
		}
		
		if (_right.value() == Bond::Broken)
		{
			assign(_left, Bond::NotAcceptor, "a broken bond on one side cannot "\
			       "be juxtaposed by an acceptor bond on the other");
		}

		// acceptor bonds cannot be paired with a non-existent half-bond on
		// the other side - formerly unpaired_left/unpaired_right
		if (_leftExist.value() == Existence::Absent &&
		    _rightExist.value() == Existence::Present)
		{
			assign(_right, Bond::NotAcceptor);
		}

		// a definite donor bond implies the protonation slot is sampled -
		// formerly bond_is_donor
		if (_leftExist.value() == Existence::Present &&
		    _left.value() == Bond::Donor)
		{
			assign(_hSlot, Existence::Present);
		}

		// a definite acceptor bond implies the opposite half-bond exists -
		// formerly bond_is_acceptor
		if (_leftExist.value() == Existence::Present &&
		    _left.value() == Bond::Acceptor)
		{
			assign(_rightExist, Existence::Present);
		}

		// --- absorbed from Coordinated_Constraints.cpp's
		// create_two_half_hydrogen_bonds (formerly separate Stricter/
		// StricterBond/SubExistence constraints) ---

		// half-bond existence <-> protonation slot (hSlot) subservience -
		// formerly SubExistence(le, hSlot, re, false)
		if (_leftExist.value() == Existence::Present &&
		    _rightExist.value() == Existence::Present)
		{
			assign(_hSlot, Existence::Present);
		}
		if (_leftExist.value() == Existence::Absent &&
		    _rightExist.value() == Existence::Absent)
		{
			assign(_hSlot, Existence::Absent, "both half-bond existences "\
			       "were absent so the protonation slot should also be "\
			       "absent");
		}
		if (_hSlot.value() == Existence::Absent)
		{
			if (_leftExist.value() & Existence::Absent)
			{
				assign(_leftExist, Existence::Absent, "the protonation "\
				       "slot was absent and left half-bond existence "\
				       "could be absent");
			}
			if (_rightExist.value() & Existence::Absent)
			{
				assign(_rightExist, Existence::Absent, "the protonation "\
				       "slot was absent and right half-bond existence "\
				       "could be absent");
			}
		}

		// lone pair / hydrogen mutual exclusion - formerly
		// lone_pair_cannot_brace_hydrogen (four call sites collapse to two
		// ifs, since two originally-separate Stricter<Existence>/
		// Stricter<Bond> instances shared an identical condition and can
		// now share one `if`)
		bool leftIsSampledLonePair = (_leftExist.value() == Existence::Present
		                              && _left.value() == Bond::LonePair);
		if (leftIsSampledLonePair && _protonated.value() == Existence::Present)
		{
			assign(_hSlot, Existence::Absent);
//			assign(_left, Bond::NotLonePair);
		}
		if (leftIsSampledLonePair && _hSlot.value() == Existence::Present)
		{
			assign(_protonated, Existence::Absent);
		}

		// formerly hydrogen_cannot_brace_lonepair - distinct condition
		// (doesn't check _left's current value at all, unlike the block
		// above)
		if (_leftExist.value() == Existence::Present &&
		    _hSlot.value() == Existence::Present &&
		    _protonated.value() == Existence::Present)
		{
			assign(_left, Bond::NotLonePair);
		}

		// an unsampled half-bond whose other side isn't a donor/acceptor
		// either means the protonation slot is believed unsampled -
		// formerly non_existent_bonds
		if ((_leftExist.value() == Existence::Absent &&
		     !(_right.value() & Bond::Bonded)) ||
		    (_rightExist.value() == Existence::Absent &&
		     !(_left.value() & Bond::Bonded)))
		{
			assign(_hSlot, Existence::Absent);
		}

		// if every part of the H-bond is definitely sampled and the other
		// side is definitely bonded, this side cannot be broken - formerly
		// could_be_bonded
		if (_leftExist.value() == Existence::Present &&
		    _rightExist.value() == Existence::Present &&
		    _protonated.value() == Existence::Present &&
		    _right.value() == Bond::Bonded)
		{
//			assign(_left, Bond::NotBroken);
		}

		return assign.okay();
	}

	BondConnector &_left;
	ExistenceConnector &_leftExist;
	ExistenceConnector &_protonated;
	ExistenceConnector &_hSlot;
	BondConnector &_right;
	ExistenceConnector &_rightExist;
};
};


#endif
