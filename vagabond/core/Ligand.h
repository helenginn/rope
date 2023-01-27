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

#ifndef __vagabond__Ligand__
#define __vagabond__Ligand__

#include <string>
#include <set>
#include "Instance.h"
#include "ResidueId.h"

class Substance;
class AtomGroup;

class Ligand : public Instance
{
public:
	Ligand();
	Ligand(std::string model_id, AtomGroup *grp);

	const std::string &substance_id() const
	{
		return _substance_id;
	}

	virtual std::string desc() const;
	
	Substance *const substance();
protected:
	virtual bool atomBelongsToInstance(Atom *a);
private:
	std::string _anchorDesc;
	std::string _nickname;
	std::string _chain;
	std::set<ResidueId> _resids;

	std::string _substance_id;
	Substance *_substance = nullptr;
};

#endif
