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

#include "BreakMatrix.h"
#include "ConstraintBase.h"
#include "Coordinated.h"

using namespace hnet;

BreakMatrix::BreakMatrix(Coordinated *coord, const ConnectMap &bonds,
                         const OpSet<AcceptableGroup> &groups,
                         CountConnector &unbroken_count)
: _coord(coord), _ac(coord->atomConf()), _bonds(bonds), 
_unbrokenCount(unbroken_count), _myExist(*coord->existence())
{
	setup(groups);
}

void BreakMatrix::insertGroupIntoMatrix(const AcceptableGroup &group)
{
	for (const ABPair &left : group.group)
	{
		if (_indexing.count(left.second) == 0)
		{
			// likely an uninvolved coordinator
			continue;
		}

		int li = _indexing[left.second];
		for (const ABPair &right : group.group)
		{
			if (_indexing.count(right.second) == 0)
			{
				// likely an uninvolved coordinator
				continue;
			}

			int ri = _indexing[right.second];

			_matrix(li, ri) = 1;
			_matrix(ri, li) = 1;
		}
	}
}

void BreakMatrix::accounting()
{
	int n = _bonds.size();
	int count = 0;

	for (auto it = _bonds.begin(); it != _bonds.end(); it++)
	{
		if (it->first)
		{
			_indexing[it->first] = count++;
			
			BreakEntry be;
			be.bond = it->first;
			be.exist = it->second;
			be.fake = true;
			if (_coord->bond2HydrogenSample().count(be.bond))
			{
				be.fake = false;
			}
			be.index = count;

			for (const ABPair &ab : _coord->bonds())
			{
				if (ab.second != it->first)
				{
					continue;
				}
				Coordinated *other = _coord->atomMap()[ab.first];
				break;
			}
			
			_entries.push_back(be);
		}
	}

	_matrix = Eigen::MatrixXi(n, n);
	_matrix.setIdentity();
}

void BreakMatrix::checks_forgets()
{
	std::vector<ConnectBase *> connectors;
	connectors.push_back(&_myExist);
	connectors.push_back(&_unbrokenCount);
	for (const BreakEntry &entry : _entries)
	{
		connectors.push_back(entry.bond);
		connectors.push_back(entry.exist);
	}
	
	prep_constraints_and_forgets(this, connectors);
}

std::string wrap(const std::string &reason)
{
	if (ConnectBase::_silent) 
	{
		return {};
	}
	else return reason;
}

void BreakMatrix::setup(const OpSet<AcceptableGroup> &groups)
{
	accounting();
	
	for (const AcceptableGroup &group : groups)
	{
		insertGroupIntoMatrix(group);
	}

	std::vector<BondConnector *> all;
	for (auto &pair : _indexing)
	{
		all.push_back(pair.first);
	}

	std::cout << "Coordination's (" << _ac.desc() << ") break matrix: " << std::endl;
	print_current(all);
	
	checks_forgets();
}

Eigen::MatrixXi BreakMatrix::partialMatrix(const std::vector<BondConnector *> 
                                           &partial)
{
	std::map<BondConnector *, int> lookup;
	for (int i = 0; i < partial.size(); i++)
	{
		lookup[partial[i]] = i;
	}

	Eigen::MatrixXi tmp(partial.size(), partial.size());
	tmp.setZero();

	for (BondConnector *const &left : partial)
	{
		if (_indexing.count(left) == 0)
		{
			// likely an uninvolved coordinator
			continue;
		}
		int li = _indexing[left];
		int lj = lookup[left];
		for (BondConnector *const &right : partial)
		{
			if (_indexing.count(right) == 0)
			{
				// likely an uninvolved coordinator
				continue;
			}

			int ri = _indexing[right];
			int rj = lookup[right];
			
			tmp(lj, rj) = _matrix(li, ri);
		}
	}

	return tmp;
}

void BreakMatrix::print_current(const std::vector<BondConnector *> &in_game)
{
	return;
	if (ConnectBase::_silent)
	{
		return;
	}

	Eigen::MatrixXi tmp = partialMatrix(in_game);
	
	ConnectBase::out() << _myExist << " working on matrix: " << std::endl;
	for (int i = 0; i < in_game.size(); i++)
	{
		ConnectBase::out() << tmp.row(i) << " <- " << *in_game[i] << " (" <<
		in_game[i]->value() << ")" << std::endl;
	}

	ConnectBase::out() << std::endl;
}

bool BreakMatrix::break_others(hnet::make_assign_and_say<BreakMatrix> &assign, 
                               BreakEntry &definite)
{
	BondConnector *bond = definite.bond;
	int row = _indexing[bond];

	Eigen::VectorXi break_list = _matrix(row, Eigen::all);
	
	for (int i = 0; i < break_list.size(); i++)
	{
		if (break_list[i] == 1)
		{
			continue; // safe from being broken;
		}

		BreakEntry &other = _entries[i];
		std::string str = {};
		if (!ConnectBase::_silent)
		{
			std::ostringstream ss; 
			ss << "breaking antagonists of " << *bond;
			str = ss.str();
		}
		if (assertAbsence(assign, other, str))
		{
			return true;
		}
	}
	return false;
}

Existence::Values BreakMatrix::BreakEntry::existence()
{
	return (exist ? exist->value() : Existence::Unassigned);
}

bool BreakMatrix::evaluate(hnet::make_assign_and_say<BreakMatrix> &assign)
{
//	std::cout << " == Evaluation round == " << std::endl;

	/* stage one: gather bonds of interest, break competing bonds 
	 * where an unbroken/present bond is identified. */
	std::vector<BondConnector *> in_game;
	std::vector<BondConnector *> nonfake_in;

	for (BreakEntry &entry : _entries)
	{
		Bond::Values obj = entry.bond->value();
		Existence::Values exist = entry.existence();
		
		Attachment attach(*entry.bond, *entry.exist);
		
		if (_indexing.count(entry.bond) == 0)
		{
			continue;
		}
		
		// definitely not broken, definitely exists

		CountType type = attach.type_of_bond(Bond::NotBroken);
		if (type == Certain)
		{
			if (break_others(assign, entry))
			{
				return true;
			}
		}

		if (type == Certain || type == Maybe)
		{
			in_game.push_back(entry.bond);
			if (!entry.fake)
			{
				nonfake_in.push_back(entry.bond);
			}
		}
	}
	
	// acquire lowest allowed broken bonds to analyse gathered bonds.
	Count::Values unbrokens = _unbrokenCount.value();
	std::vector<int> options = possible_values(unbrokens);
	
	if (options.size() == 0)
	{
		return false; // we have contradiction anyway, stop calculating
	}
	
	int lower_limit = options[0];
	if (!ConnectBase::_silent)
	{
		ConnectBase::out() << "Lower limit for " << _ac.desc() << ": " << lower_limit << std::endl;
		print_current(in_game);
	}

	Eigen::MatrixXi tmp = partialMatrix(in_game);

	int max_sum = 0;
	int min_sum = INT_MAX;
	for (int i = 0; i < in_game.size(); i++)
	{
		max_sum = std::max(max_sum, tmp(i, Eigen::all).sum());
		min_sum = std::min(min_sum, tmp(i, Eigen::all).sum());
	}

	/* break rows where the configurations available cannot reach 
	 * the lower limit
	 * 	looking for instances of e.g. 1 1 0 0 0 where min from bond count is 3
	 *  */
	for (int i = 0; i < in_game.size(); i++)
	{
		int total = tmp(i, Eigen::all).sum();
		if (total < lower_limit && lower_limit < in_game.size() && 
		    max_sum >= lower_limit)
		{
			int idx = _indexing[in_game[i]];
			if (assertAbsence(assign, _entries[idx], 
			                  wrap("this configuration has too "\
			                  "few partners to complete the coordination")))
			{
				return true;
			}
		}
	}
	
	/* check whether any rows are purely non-broken, in which case there is
	 * no configuration where this bond cannot be included, therefore the
	 * bond must be unbroken
	 * 	looking for instances of e.g. 1 1 1 1 with no zeros
	 *  */
	for (int i = 0; i < in_game.size(); i++)
	{
		int total = tmp(i, Eigen::all).sum();
		int idx = _indexing[in_game[i]];
		BreakEntry &entry = _entries[idx];
		Attachment attach(*entry.bond, *entry.exist);

		if (total == in_game.size())
		{
			//std::cout << "Removing possibility of broken for " << i << std::endl;
			CountType certainty = (_myExist.value() == Existence::Present ?
			                       Certain : Maybe);
			
			if (entry.exist->value() == Existence::Unassigned)
			{
				certainty = Maybe;
			}

			if (attach.inform(Bond::NotBroken, assign, certainty))
			{
				return true;
			}
		}
	}
	
	return false;
}

bool BreakMatrix::assertAbsence(make_assign_and_say<BreakMatrix> &assign, 
                                 BreakEntry &chosen, std::string reason)
{
	if (_myExist.value() != Existence::Present)
	{
		return false;
	}

	int idx = _indexing[chosen.bond];
	Attachment attach(*chosen.bond, *chosen.exist);
	bool changed = attach.inform(Bond::Broken, assign, Certain);
	return changed;
}

bool BreakMatrix::check(const GuiltVersion &gv, CheckList &list)
{
	bool changed = false;

	auto assign = make_assign_and_say(this, gv, list);

	// don't check anything if we may be non-existent
	if (_myExist.value() == Existence::Absent)
	{
		return assign.okay();
	}

	do
	{
		changed = evaluate(assign);
		if (!assign.okay())
		{
			return false;
		}
	}
	while (changed);

//	std::cout << "Done." << std::endl;
//	std::cout << std::endl;
	
	return assign.okay();
}
