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

#include "ContactPoint.h"
#include <vagabond/core/Model.h>
#include "Mab.h"

ContactPoint::ContactPoint(Fiducial &fiducial, Antigens &antigens)
: _fiducial(fiducial), _antigens(antigens)
{
	OpSet<std::string> fid_ents = 
	fiducial.non_antigen_entities(antigens);

	_chosen = antigens.antigen(fiducial.antigen);
	
	if (!_chosen)
	{
		throw std::runtime_error("Missing antigen for antibody " 
		                         + fiducial.name);
	}

	OpSet<std::string> antigen_ents = _chosen->entities;

	std::vector<Instance *> fid_insts = fiducial.model.instances();
	
	for (Instance *const &inst : fid_insts)
	{
		std::string name = inst->entity_id();
		if (fid_ents.count(name))
		{
			_iFiducials.push_back(inst);
		}
		else if (antigen_ents.count(name))
		{
			_iFidAntigens.push_back(inst);
		}
	}

	std::vector<Instance *> gen_insts = _chosen->model.instances();
	for (Instance *const &inst : gen_insts)
	{
		std::string name = inst->entity_id();
		if (antigen_ents.count(name))
		{
			_iAntigens.push_back(inst);
		}
	}
	
	std::cout << "Antibody instances: " << _iFiducials.size() << std::endl;
	std::cout << "Antibody's antigen instances: " << _iFidAntigens.size() << std::endl;
	std::cout << "Antigens's antigen instances: " << _iAntigens.size() << std::endl;
}
