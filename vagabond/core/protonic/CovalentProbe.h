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

#ifndef __vagabond__CovalentProbe__
#define __vagabond__CovalentProbe__

#include "Probe.h"
#include "BondAngle.h"

inline hnet::AtomConf find_partner(hnet::AtomConf atom,
                                   const std::string &search)
{
	hnet::AtomConf partner = {};
	char conf = atom.conf;
	for (size_t i = 0; i < atom.ptr->bondAngleCount(); i++)
	{
		BondAngle *angle = atom.ptr->bondAngle(i);
		if (angle->atom(0)->atomName() == search)
		{
			partner = {angle->atom(0), conf};
		}
		if (angle->atom(2)->atomName() == search)
		{
			partner = {angle->atom(2), conf};
		}

		if (partner.ptr) break;
	}

	return partner;
}

inline
hnet::Covalent::Values covalent_status_for_bond(const hnet::AtomConf &left,
                                          const hnet::AtomConf &right)
{
	auto either_are_named_couple = []
	(const std::string &a, const std::string &b)
	{
		return [a, b](hnet::AtomConf left, hnet::AtomConf right) -> bool
		{
			return (left.ptr->atomName() == a && right.ptr->atomName() == b) ||
			        (right.ptr->atomName() == a && left.ptr->atomName() == b);
		};
	};

	bool sure_double = false;
	if (left.ptr->code() == "ASN" || left.ptr->code() == "GLN")
	{
		sure_double |= either_are_named_couple("CG", "OD1")(left, right);
		sure_double |= either_are_named_couple("CD", "OE1")(left, right);
	}
	if (left.ptr->code() == "TYR" || left.ptr->code() == "PHE")
	{
		sure_double |= either_are_named_couple("CG", "CD1")(left, right);
		sure_double |= either_are_named_couple("CD2", "CE2")(left, right);
		sure_double |= either_are_named_couple("CE1", "CZ")(left, right);
	}
	if (left.ptr->code() == "TRP")
	{
		sure_double |= either_are_named_couple("CG", "CD1")(left, right);
		sure_double |= either_are_named_couple("CE2", "CZ2")(left, right);
		sure_double |= either_are_named_couple("CH2", "CZ3")(left, right);
		sure_double |= either_are_named_couple("CE3", "CD2")(left, right);
	}
	
	if (sure_double)
	{
		return hnet::Covalent::Double;
	}

	bool maybe = false;
	maybe |= either_are_named_couple("OXT", "C")(left, right);
	
	if (either_are_named_couple("C", "O")(left, right))
	{
		hnet::AtomConf o = (left.ptr->atomName() == "O" ? left : right);
		std::cout << "We got to this point for " << o.ptr->desc() << std::endl;
		hnet::AtomConf oxt = find_partner(o, "OXT");
		std::cout << "oxt is: " << oxt << std::endl;
		if (oxt.ptr)
		{
			maybe = true;
		}
		else
		{
			return hnet::Covalent::Double;
		}
	}

	if (left.ptr->code() == "ASP" || left.ptr->code() == "GLU")
	{
		maybe |= either_are_named_couple("CG", "OD1")(left, right);
		maybe |= either_are_named_couple("CD", "OE1")(left, right);
		maybe |= either_are_named_couple("CG", "OD2")(left, right);
		maybe |= either_are_named_couple("CD", "OE2")(left, right);
	}
	if (left.ptr->code() == "ARG")
	{
		maybe |= either_are_named_couple("NH2", "CZ")(left, right);
		maybe |= either_are_named_couple("NH1", "CZ")(left, right);
		maybe |= either_are_named_couple("NE", "CZ")(left, right);
	}
	if (left.ptr->code() == "HIS")
	{
		maybe |= either_are_named_couple("CE1", "ND1")(left, right);
		maybe |= either_are_named_couple("CE1", "NE2")(left, right);
	}
	
	if (maybe)
	{
		std::cout << "Unassigned between " << left.ptr->atomName() << " and "
		<< right.ptr->atomName() << std::endl;
		return hnet::Covalent::Unassigned;
	}

	return hnet::Covalent::Single;
}
	

class CovalentProbe : public BondProbe
{
public:
	CovalentProbe(Probe &left, Probe &right, hnet::ExistenceConnector &exist,
	              hnet::CovalentConnector &cov) 
	: BondProbe(_silent, left, right, exist), _cov(cov)
	{
	}

	virtual const glm::vec3 &position() const
	{
		return _left.position();
	}
	
	const glm::vec3 &end() const
	{
		return _right.position();
	}

	virtual bool is_covalent()
	{
		return true;
	}

	virtual bool is_certain()
	{
		return _exist.is_certain();
	}


	virtual bool is_text()
	{
		return false;
	}

	virtual std::string desc()
	{
		return "covalent bond between " + _left.desc() + " and " + _right.desc();
	}
	
	virtual std::string display()
	{
		std::string str;
		hnet::Covalent::Values val = _cov.value(true);

		switch (val)
		{
			case hnet::Covalent::Double:
			str = "double_bond";
			break;

			case hnet::Covalent::Single:
			str = "single_bond";
			break;

			case hnet::Covalent::Unassigned:
			str = "single_or_double_bond";
			break;

			default:
			str = "unassigned_bond";
			break;
		}
		
		return str;
	}

	bool _doubleBond{false};
	hnet::CovalentConnector &_cov;
	hnet::BondConnector _silent{};
};


#endif
