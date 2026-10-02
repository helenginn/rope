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
			be.index = count;

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

std::string wrap(const char *reason)
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

	print_current(all);
	
	checks_forgets();
}

Eigen::MatrixXi BreakMatrix::partialMatrix(const std::vector<BondConnector *> 
                                           &partial)
{
	std::vector<int> idx(partial.size(), -1);
	bool all_found = true;

	for (size_t i = 0; i < partial.size(); i++)
	{
		auto it = _indexing.find(partial[i]);
		if (it == _indexing.end())
		{
			// likely an uninvolved coordinator
			all_found = false;
			continue;
		}
		idx[i] = it->second;
	}

	if (all_found)
	{
		return _matrix(idx, idx);
	}

	Eigen::MatrixXi tmp(partial.size(), partial.size());
	tmp.setZero();

	for (size_t i = 0; i < partial.size(); i++)
	{
		if (idx[i] < 0) { continue; }
		for (size_t j = 0; j < partial.size(); j++)
		{
			if (idx[j] < 0) { continue; }
			tmp(i, j) = _matrix(idx[i], idx[j]);
		}
	}

	return tmp;
}

void BreakMatrix::print_current(const std::vector<BondConnector *> &in_game)
{
	if (!(_ac.desc() == "D-HOH155:O" || _ac.desc() == "A-SER85:OG,A"))
	{
		return;
	}

	if (ConnectBase::_silent)
	{
		return;
	}

	std::cout << "Coordination's (" << _ac.desc() << ") break matrix: " << std::endl;

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

	for (int i = 0; i < _matrix.cols(); i++)
	{
		if (_matrix(row, i) == 1)
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
	/* stage one: gather bonds of interest, break competing bonds 
	 * where an unbroken/present bond is identified. */
	// reused between calls (evaluate() runs on every propagation pass)
	std::vector<BondConnector *> &in_game = _inGame;
	std::vector<int> &game_idx = _inGameIdx; // each one's row in _matrix
	in_game.clear();
	game_idx.clear();

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
			game_idx.push_back(_indexing[entry.bond]);
		}
	}
	
	// acquire lowest allowed broken bonds to analyse gathered bonds.
	Count::Values unbrokens = _unbrokenCount.value();
	
	if (unbrokens == Count::Contradiction)
	{
		return false; // we have contradiction anyway, stop calculating
	}
	
	// the first of its possible values, i.e. the one on its lowest bit -
	// the same one possible_values() puts first
	int lower_limit = count_to_int((Count::Values)(1u << 
	                               std::countr_zero((unsigned int)unbrokens)));
	if (!ConnectBase::_silent)
	{
//		ConnectBase::out() << "Lower limit for " << _ac.desc() << ": " << lower_limit << std::endl;
		print_current(in_game);
	}

	// the sub-matrix over in_game, read in place from _matrix rather than
	// copied out: tmp(i, j) in what follows.
	auto tmp = [this, &game_idx](int i, int j) -> int
	{
		return _matrix(game_idx[i], game_idx[j]);
	};

	std::vector<int> &row_sum = _rowSums;
	row_sum.assign(in_game.size(), 0);

	int max_sum = 0;
	for (int i = 0; i < in_game.size(); i++)
	{
		int sum = 0;
		for (int j = 0; j < in_game.size(); j++)
		{
			sum += tmp(i, j);
		}

		row_sum[i] = sum;
		max_sum = std::max(max_sum, sum);
	}

	/* break rows where the configurations available cannot reach 
	 * the lower limit
	 * 	looking for instances of e.g. 1 1 0 0 0 where min from bond count is 3
	 *  */
	for (int i = 0; i < in_game.size(); i++)
	{
		int total = row_sum[i];
		if (total < lower_limit && lower_limit < in_game.size() &&
		    max_sum >= lower_limit)
		{
			int idx = game_idx[i];
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
		int total = row_sum[i];
		int idx = game_idx[i];
		BreakEntry &entry = _entries[idx];
		Attachment attach(*entry.bond, *entry.exist);

		if (total == in_game.size() && total == lower_limit)
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

	/*  if there are non-placeholder bonds available, and they would have 
	 *  to be Broken in order to be missing, and there are placeholder 
	 *  hydrogens which could be missing instead, then get rid of the 
	 * 	placeholder hydrogens */
	std::vector<int> &reference = _reference;
	for (int i = 0; i < in_game.size() && 
	     _myExist.value() == Existence::Present; i++)
	{
		int idx = game_idx[i];
		BreakEntry &entry = _entries[idx];
		Attachment attach(*entry.bond, *entry.exist);
		
		// we do not care about this kind of "Maybe"
		/*
		if (entry.exist->value() != Existence::Present)
		{
			continue;
		}
		*/

		CountType certainty = attach.type_of_bond(Bond::NotBroken);
		if (certainty != Maybe) // due to non-existence
		{
			continue;
		}

		reference.resize(in_game.size());
		for (int k = 0; k < in_game.size(); k++)
		{
			reference[k] = tmp(i, k);
		}

		// we need to know, for each placeholder hydrogen, if it is
		// a purely direct swap for the current bond (scrap), or if it is
		// capable of co-existing with other bonds (allowed)

		for (int j = 0; j < in_game.size(); j++)
		{
			int jdx = game_idx[j];
			BreakEntry &candidate = _entries[jdx];
			// ignore non-placeholder
			if (!candidate.bond->_placeholder) { continue; }

			// if the two bonds can co-exist, we are not interested
			if (tmp(j, i) == 1)
			{
				continue;
			}
			
			// temporarily set diagonal elements to zero and check the diff.
			// (reference carries over between candidates of the same i, as
			// it always has: it is only reloaded per bond above.)
			reference[i] = 0;
			
			int total = 0;
			for (int k = 0; k < in_game.size(); k++)
			{
				int compare = (k == j ? 0 : tmp(j, k));
				reference[k] -= compare;
				total += std::abs(reference[k]);
			}
			
			if (total == 0)
			{
				Attachment rid_candidate(*candidate.bond, *candidate.exist);
				if (rid_candidate.inform(Bond::Broken, assign, Certain))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

bool BreakMatrix::assertAbsence(make_assign_and_say<BreakMatrix> &assign,
                                 BreakEntry &chosen, const std::string &reason)
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
