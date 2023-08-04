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

#include "PolymerEntity.h"
#include "Sequence.h"

PolymerEntity::PolymerEntity() : Entity()
{

}

Metadata *PolymerEntity::funcBetweenAtoms(const std::vector<Atom3DPosition> &ps,
                                          const std::string &header,
                                          const Calculate &calculate)
{
	Metadata *md = new Metadata();
	
	for (Model *model : _models)
	{
		model->load(Model::NoGeometry);
		std::vector<Instance *> instances = model->instances();
		for (Instance *inst : instances)
		{
			if (inst->entity() != this)
			{
				continue;
			}
			
			std::vector<Atom *> atoms(ps.size());
			
			bool valid = true;
			for (size_t i = 0; i < ps.size(); i++)
			{
				atoms[i] = inst->atomForIdentifier(ps[i]);
				if (atoms[i] == nullptr)
				{
					valid = false;
					break;
				}
			}
			
			if (valid)
			{
				Metadata::KeyValues kv;

				kv["instance"] = Value(inst->id());
				kv[header] = Value(f_to_str(calculate(atoms), 2));
				md->addKeyValues(kv, true);
			}
		}

		md->clickTicker();
		model->unload();
	}

	md->finishTicker();

	return md;
}

const bool compare_id(const Polymer *a, const Polymer *b)
{
	return a->id() > b->id();
}

void PolymerEntity::housekeeping()
{
	std::sort(_molecules.begin(), _molecules.end(), compare_id);
	Entity::housekeeping();
	_sequence.setEntity(this);
}

MetadataGroup PolymerEntity::prepareTorsionGroup()
{
	size_t num = sequence()->torsionCount();
	std::vector<ResidueTorsion> headers;
	_sequence.addResidueTorsions(headers);

	MetadataGroup group(num);
	group.addHeaders(headers);

	return group;
}

PositionalGroup PolymerEntity::preparePositionGroup()
{
	std::vector<Atom3DPosition> headers;
	_sequence.addAtomPositionHeaders(headers);
	
	PositionalGroup group(headers.size());
	group.addHeaders(headers);

	return group;
}

void PolymerEntity::throwOutInstance(Polymer *mol)
{
	mol->eraseIfPresent(_molecules);
}

const std::vector<Instance *> PolymerEntity::instances() const
{
	std::vector<Instance *> instances;
	instances.reserve(_molecules.size());
	
	for (Polymer *p : _molecules)
	{
		instances.push_back(p);
	}
	
	return instances;
}

void PolymerEntity::appendIfMissing(Instance *inst)
{
	inst->appendIfMissing(_molecules);
}
