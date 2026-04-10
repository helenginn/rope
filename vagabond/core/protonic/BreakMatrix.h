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
#include "ConstraintBase.h"
#include "Connector.h"
#include <vagabond/utils/Eigen/Dense>

namespace hnet
{
class Coordinated;
typedef std::map<BondConnector *, ExistenceConnector *> ConnectMap;

class BreakMatrix : public ConstraintBase
{
public:
	BreakMatrix(Coordinated *coord, const ConnectMap &bonds,
	            const OpSet<AcceptableGroup> &groups,
	            CountConnector &unbroken_count,
	            CountConnector &twirling_bonds);
	
	std::string desc()
	{
		return "Break matrix for " + _ac.desc();
	}

	void setup(const OpSet<AcceptableGroup> &groups);
	void insertGroupIntoMatrix(const AcceptableGroup &group);

	bool check(const GuiltVersion &gv, CheckList &list);
private:
	void accounting();
	bool evaluate(make_assign_and_say<BreakMatrix> &assign); 
	void checks_forgets();
	bool assertExistence(make_assign_and_say<BreakMatrix> &assign, 
	                    BondConnector *chosen);
	bool assertAbsence(make_assign_and_say<BreakMatrix> &assign, 
	                   BondConnector *chosen, std::string reason = {});
	Eigen::MatrixXi partialMatrix(const std::vector<BondConnector *> 
	                              &partial);
	bool break_others(make_assign_and_say<BreakMatrix> &assign, 
	                  BondConnector *definite);

	const Coordinated *_coord{};
	const AtomConf &_ac;
	ConnectMap _bonds;
	CountConnector &_unbrokenCount;
	ExistenceConnector &_myExist;
	CountConnector &_twirling;
	std::map<BondConnector *, int> _indexing{};
	Eigen::MatrixXi _matrix{};
	
	struct BreakEntry
	{
		BondConnector *bond{};
		ExistenceConnector *exist{};
		ExistenceConnector *partner{};
		ExistenceConnector *hSample{};
		bool fake;
		int index{};
		
		Existence::Values existence();
	};
	
	std::vector<BreakEntry> _entries;
};
};

#endif
