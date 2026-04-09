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

#ifndef __vagabond__BreakMatrix__
#define __vagabond__BreakMatrix__

#include "alignment.h"
#include "Connector.h"
#include <vagabond/utils/Eigen/Dense>

namespace hnet { class Coordinated; };

typedef std::map<hnet::BondConnector *, hnet::ExistenceConnector *> ConnectMap;
class BreakMatrix
{
public:
	BreakMatrix(hnet::Coordinated *coord, const ConnectMap &bonds,
	            const OpSet<hnet::AcceptableGroup> &groups,
	            hnet::CountConnector &unbroken_count,
	            hnet::CountConnector &twirling_bonds);
	
	std::string desc()
	{
		return "Break matrix for " + _ac.desc();
	}

	void setup(const OpSet<hnet::AcceptableGroup> &groups);
	void insertGroupIntoMatrix(const hnet::AcceptableGroup &group);

	void forget(const GuiltVersion &gv);

	bool check(const GuiltVersion &gv, hnet::CheckList &list);
private:
	void accounting();
	bool evaluate(hnet::make_assign_and_say<BreakMatrix> &assign); 
	void checks_forgets();
	bool assertExistence(hnet::make_assign_and_say<BreakMatrix> &assign, 
	                    hnet::BondConnector *chosen);
	bool assertAbsence(hnet::make_assign_and_say<BreakMatrix> &assign, 
	                   hnet::BondConnector *chosen, std::string reason = {});
	Eigen::MatrixXi partialMatrix(const std::vector<hnet::BondConnector *> 
	                              &partial);
	bool break_others(hnet::make_assign_and_say<BreakMatrix> &assign, 
	                  hnet::BondConnector *definite);

	const hnet::Coordinated *_coord{};
	const hnet::AtomConf &_ac;
	ConnectMap _bonds;
	hnet::CountConnector &_unbrokenCount;
	hnet::ExistenceConnector &_myExist;
	hnet::CountConnector &_twirling;
	std::map<hnet::BondConnector *, int> _indexing{};
	Eigen::MatrixXi _matrix{};
	
	struct BreakEntry
	{
		hnet::BondConnector *bond{};
		hnet::ExistenceConnector *exist{};
		hnet::ExistenceConnector *partner{};
		hnet::ExistenceConnector *hSample{};
		bool fake;
		int index{};
		
		hnet::Existence::Values existence();
	};
	
	std::vector<BreakEntry> _entries;
};

#endif
