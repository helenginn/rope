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
                         CountConnector &unbroken_count,
                         CountConnector &twirling_bonds)
: _coord(coord), _ac(coord->atomConf()), _bonds(bonds), 
_unbrokenCount(unbroken_count), _myExist(*coord->existence()),
_twirling(twirling_bonds)
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
				be.hSample = _coord->bond2HydrogenSample().at(be.bond);
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
				be.partner = other->existence();
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
	for (const BreakEntry &entry : _entries)
	{
		connectors.push_back(entry.bond);
		connectors.push_back(entry.exist);
		connectors.push_back(entry.partner);
		connectors.push_back(entry.hSample);

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

	std::cout << "Coordination's (" << _ac.desc() << ") break matrix: " << std::endl;
	std::cout << _matrix << std::endl;
	
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

bool BreakMatrix::break_others(hnet::make_assign_and_say<BreakMatrix> &assign, 
                               BondConnector *definite)
{
	int row = _indexing[definite];

	Eigen::VectorXi break_list = _matrix(row, Eigen::all);
//	std::cout << "Breaking antagonists of " << *definite << std::endl;
	
	for (int i = 0; i < break_list.size(); i++)
	{
		if (break_list[i] == 1)
		{
			continue; // safe from being broken;
		}

		BondConnector *other = _entries[i].bond;
		std::string str = {};
		if (!ConnectBase::_silent)
		{
			std::ostringstream ss; 
			ss << "breaking antagonists of " << *definite;
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
		
		if (_indexing.count(entry.bond) == 0)
		{
			continue;
		}
		
		// definitely not broken, definitely exists
		if ((obj & Bond::NotBroken && !(obj & Bond::Broken))
		    && (exist == Existence::Present))
		{
			if (break_others(assign, entry.bond))
			{
				return true;
			}
		}

		if ((obj != Bond::Broken) && (exist & Existence::Present))
		{
			//std::cout << "Bond value desc: " << entry.bond->desc() << " ";
			//std::cout << exist << std::endl;
			in_game.push_back(entry.bond);
			if (!entry.fake)
			{
				nonfake_in.push_back(entry.bond);
			}
			
			if (assertExistence(assign, entry.bond))
			{
				return true;
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

	Eigen::MatrixXi tmp = partialMatrix(in_game);
	/*
	std::cout << _myExist << " working on matrix: " << std::endl;
	for (int i = 0; i < in_game.size(); i++)
	{
		std::cout << tmp.row(i) << " <- " << *in_game[i] << std::endl;
	}

	std::cout << "Lower limit: " << lower_limit << std::endl;
	*/

	int max_sum = 0;
	int min_sum = INT_MAX;
	for (int i = 0; i < in_game.size(); i++)
	{
		max_sum = std::max(max_sum, tmp(i, Eigen::all).sum());
		min_sum = std::min(min_sum, tmp(i, Eigen::all).sum());
	}
	if ((lower_limit > 1 || min_sum > 1) &&
	    _twirling.value() != Count::Zero)
	{
		return assign(_twirling, Count::Zero, 
		              wrap("lower limit of explicit bonds "\
		              "is >2, twirling is not required"));
	}
	
	/* break rows where the configurations available cannot reach 
	 * the lower limit */
	for (int i = 0; i < in_game.size(); i++)
	{
		int total = tmp(i, Eigen::all).sum();
		if (total < lower_limit && lower_limit < in_game.size() && 
		    max_sum >= lower_limit)
		{
			//std::cout << "Breaking row " << i << "!" << std::endl;
			if (assertAbsence(assign, in_game[i], 
			                  wrap("this configuration has too "\
			                  "few partners to complete the coordination")))
			{
				return true;
			}
		}
	}
	
	/* check whether any rows are purely non-broken, in which case there is
	 * no configuration where this bond cannot be included, therefore the
	 * bond must be unbroken */
	for (int i = 0; i < in_game.size(); i++)
	{
		int total = tmp(i, Eigen::all).sum();
		int idx = _indexing[in_game[i]];
		if (total == in_game.size())
		{
			//std::cout << "Removing possibility of broken for " << i << std::endl;
			if (assign(*in_game[i], Bond::NotBroken))
			{
				return true;
			}

			// this only changes things if the bond is also sampled
			if (assertExistence(assign, in_game[i]))
			{
				return true;
			}
		}
	}

	/* next we check for hydrogen bonds to non-existent partners. These
	 * are sometimes geometrically necessary even if the partner isn't there
	 * and isn't involved. However, if there is an equivalent row in the matrix
	 * for a partner which may exist instead, then that will always be a
	 * preferred hydrogen bond. Therefore we break the hydrogen bond 
	 * in question */
	for (int i = 0; i < in_game.size(); i++)
	{
		int l_idx = _indexing[in_game[i]];
		if (_entries[l_idx].partner->value() != Existence::Absent)
		{
			continue;
		}

		Eigen::VectorXi ref = tmp.row(i);
		for (int j = 0; j < in_game.size(); j++)
		{
			if (i == j) continue;
			int r_idx = _indexing[in_game[j]];
			
			if (_entries[r_idx].partner == &_myExist)
			{
				continue;
			}

			// non-existent partners are just as uninformative.
			if (!(_entries[r_idx].partner->value() & Existence::Present))
			{
				continue;
			}

			Eigen::VectorXi compare = tmp.row(j);
			compare -= ref;
			float result = 0;
			for (float f : compare)
			{
				result += f * f;
			}

			if (result < 1e-6)
			{
				continue;
				if (assertAbsence(assign, in_game[i], 
				                  wrap("there was an equivalent "\
				                  "hydrogen-bonding pattern with an existent"\
				                  " partner")))
				{
					return true;
				}
			}
		}
	}

	Eigen::MatrixXi unfake = partialMatrix(nonfake_in);
//	std::cout << "Working on non-fake matrix: " << std::endl << 
//	unfake << std::endl;
//	std::cout << "We are currently: " << assign.okay() << std::endl;
	
	// the non-fake set works, without having to consider fake entries
	if (unfake.rows() == lower_limit && options.size() == 1 &&
	    unfake.array().sum() == lower_limit * lower_limit)
	{
//		std::cout << "Throwing out fakes." << std::endl;
		for (int i = 0; i < in_game.size(); i++)
		{
			int idx = _indexing[in_game[i]];
			if (_entries[idx].fake)
			{
				if (assertAbsence(assign, in_game[i], 
				                  wrap("the entirety of the "\
				"remaining non-fake coordination perfectly matches, so fake Hs"\
				"are now to be broken")))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

bool BreakMatrix::assertAbsence(make_assign_and_say<BreakMatrix> &assign, 
                                 BondConnector *chosen, std::string reason)
{
	int idx = _indexing[chosen];
	ExistenceConnector *hExist = _entries[idx].hSample;
	if (!hExist)
	{
		if (assign(*chosen, Bond::Broken, wrap(reason)))
		{
			return true;
		}
	}
	else if (hExist->value() & Existence::Absent)
	{
		bool changed = false;
		changed |= assign(*_entries[idx].exist, Existence::Absent, wrap(reason));

		if (changed)
		{
			return true;
		}
	}

	return false;
}

bool BreakMatrix::assertExistence(make_assign_and_say<BreakMatrix> &assign, 
                                 BondConnector *chosen)
{
	if (chosen->value() & Bond::Broken)
	{
		return false;
	}
	if (_myExist.value() != Existence::Present)
	{
		return false;
	}

	int idx = _indexing[chosen];

	if (_entries[idx].partner && 
	    _entries[idx].partner->value() != Existence::Present)
	{
		return false;
	}

	if (assign(*_entries[idx].exist, Existence::Present))
	{
		return true;
	}

	if (_entries[idx].hSample && 
	    assign(*_entries[idx].hSample, Existence::Present))
	{
		return true;
	}

	return false;
}

bool BreakMatrix::check(const GuiltVersion &gv, CheckList &list)
{
	bool changed = false;

//	std::cout << std::endl;
//	std::cout << "Checking break matrix for " << _ac.desc() << std::endl;

	auto assign = make_assign_and_say(this, gv, list);

	// don't check anything if we are non-existent
	if (_myExist.value() == Existence::Absent)
	{
		return assign.okay();
	}

	do
	{
		changed = evaluate(assign);
//		std::cout << "Changed " << changed << ", okay " << assign.okay() 
//		<< std::endl;
		if (!assign.okay())
		{
//			std::cout << "We have a contradiction; abort early." << std::endl;
			return false;
		}
	}
	while (changed);

//	std::cout << "Done." << std::endl;
//	std::cout << std::endl;
	
	return assign.okay();
}
