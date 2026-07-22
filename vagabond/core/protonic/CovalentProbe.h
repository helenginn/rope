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

	bool sure_double = either_are_named_couple("C", "O")(left, right);
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
	
	if (sure_double)
	{
		return hnet::Covalent::Double;
	}

	bool maybe = false;
	if (left.ptr->code() == "ASP" || left.ptr->code() == "GLU")
	{
		maybe |= either_are_named_couple("CG", "OD1")(left, right);
		maybe |= either_are_named_couple("CD", "OE1")(left, right);
		maybe |= either_are_named_couple("CG", "OD2")(left, right);
		maybe |= either_are_named_couple("CD", "OE2")(left, right);
	}
	
	if (maybe)
	{
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
		bool accessed = false;
		hnet::Covalent::Values val = _cov.value(&accessed);
		if (!accessed)
		{
			return "";
		}

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
