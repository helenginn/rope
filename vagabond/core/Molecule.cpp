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

#include "Chain.h"
#include "Molecule.h"
#include "AtomContent.h"
#include "Environment.h"
#include "ModelManager.h"
#include "EntityManager.h"

Molecule::Molecule()
{

}

Molecule::Molecule(std::string model_id, std::string entity_id, 
                   Sequence *derivative)
{
	_model_id = model_id;
	_entity_id = entity_id;
	_sequence = *derivative;
	_entity = (Environment::entityManager()->entity(_entity_id));
	
	_sequence.mapFromMaster(_entity);

	housekeeping();
}

void Molecule::getTorsionRefs(Chain *ch)
{
	_sequence.remapFromMaster(_entity);

	for (size_t i = 0; i < ch->bondTorsionCount(); i++)
	{
		BondTorsion *t = ch->bondTorsion(i);

		ResidueId id = t->residueId();
		TorsionRef ref = TorsionRef(t);

		Residue *local_res = _sequence.residue(id);
		if (local_res == nullptr)
		{
			continue;
		}

		local_res->addTorsionRef(ref);
		Residue *master_res = _sequence.master_residue(local_res);

		if (master_res == nullptr || master_res->nothing())
		{
			continue;
		}

		master_res->addTorsionRef(ref);
	}
}

void Molecule::housekeeping()
{
	_model = (Environment::modelManager()->model(_model_id));
	_entity = (Environment::entityManager()->entity(_entity_id));

	_sequence.remapFromMaster(_entity);
}


void Molecule::extractTorsionAngles(AtomContent *atoms)
{
	Chain *ch = atoms->chain(_chain_id);

	for (size_t i = 0; i < ch->bondTorsionCount(); i++)
	{
		BondTorsion *t = ch->bondTorsion(i);
		
		ResidueId id = t->residueId();
		std::string desc = t->desc();
		
		Residue *local = _sequence.residue(id);
		if (local == nullptr)
		{
			continue;
		}

		double angle = t->measurement(BondTorsion::SourceDerived);
		
		local->supplyRefinedAngle(desc, angle);
	}

}
