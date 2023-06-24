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

Metadata *PolymerEntity::angleBetweenAtoms(AtomRecall a, AtomRecall b,
                                           AtomRecall c)
{
	Metadata *md = new Metadata();
	
	std::string header;
	header += a.master->id().as_string() + a.atom_name;
	header += " through ";
	header += b.master->id().as_string() + b.atom_name;
	header += " to ";
	header += c.master->id().as_string() + c.atom_name;
	
	AtomRecall new_a = AtomRecall(_sequence.residueLike(a.master->id()),
	                              a.atom_name);
	AtomRecall new_b = AtomRecall(_sequence.residueLike(b.master->id()),
	                              b.atom_name);
	AtomRecall new_c = AtomRecall(_sequence.residueLike(c.master->id()),
	                              c.atom_name);

	for (Model *model : _models)
	{
		model->angleBetweenAtoms(this, new_a, new_b, new_c, header, md);
		md->clickTicker();
	}

	md->finishTicker();

	return md;
}

Metadata *PolymerEntity::distanceBetweenAtoms(AtomRecall a, AtomRecall b)
{
	Metadata *md = new Metadata();
	
	std::string header;
	header += a.master->id().as_string() + a.atom_name;
	header += " to ";
	header += b.master->id().as_string() + b.atom_name;
	
	AtomRecall new_a = AtomRecall(_sequence.residueLike(a.master->id()),
	                              a.atom_name);
	AtomRecall new_b = AtomRecall(_sequence.residueLike(b.master->id()),
	                              b.atom_name);

	for (Model *model : _models)
	{
		model->distanceBetweenAtoms(this, new_a, new_b, header, md);
		md->clickTicker();
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
