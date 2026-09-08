// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "RotamerOccupancy.h"
#include "Entity.h"
#include "Instance.h"
#include "Residue.h"
#include "ResidueTorsion.h"
#include "TorsionRef.h"
#include "BondTorsion.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Progressor.h"
#include "matrix_functions.h"

#include <set>

RotamerOccupancy::RotamerOccupancy(Entity *entity) : _entity(entity)
{

}

void RotamerOccupancy::calculate(const std::vector<ResidueTorsion> &headers,
                                  Progressor *progress,
                                  std::atomic<bool> *cancelled)
{
	std::vector<Instance *> instances = _entity->instances();

	for (Instance *instance : instances)
	{
		if (cancelled && cancelled->load())
		{
			break;
		}

		measureInstance(instance, headers);

		if (progress)
		{
			progress->clickTicker();
		}
	}

	if (progress)
	{
		progress->finishTicker();
	}
}

void RotamerOccupancy::measureInstance(Instance *instance,
                                        const std::vector<ResidueTorsion> &headers)
{
	AtomGroup *atoms = instance->currentAtoms();
	if (atoms == nullptr)
	{
		return;
	}

	InstanceAngles &instanceAngles = _results[instance];

	for (const ResidueTorsion &header : headers)
	{
		ResidueTorsion local = header;
		local.attachToInstance(instance);

		BondTorsion *bt = atoms->findBondTorsion(local.torsion().desc());
		if (bt == nullptr)
		{
			continue;
		}

		std::set<std::string> labels;
		for (int i = 0; i < 4; i++)
		{
			for (const auto &pair : bt->atom(i)->conformerPositions())
			{
				labels.insert(pair.first);
			}
		}

		std::map<std::string, double> &torsionAngles =
		instanceAngles[local.torsion().desc()];

		if (labels.empty())
		{
			glm::vec3 poz[4];
			for (int i = 0; i < 4; i++)
			{
				poz[i] = bt->atom(i)->initialPosition();
			}

			torsionAngles[""] = measure_bond_torsion(poz);
			continue;
		}

		for (const std::string &label : labels)
		{
			glm::vec3 poz[4];
			for (int i = 0; i < 4; i++)
			{
				Atom *a = bt->atom(i);
				const Atom::ConformerInfo &confs = a->conformerPositions();
				auto it = confs.find(label);
				poz[i] = (it != confs.end()) ? it->second.pos.ave
				                              : a->initialPosition();
			}

			torsionAngles[label] = measure_bond_torsion(poz);
		}
	}
}

bool RotamerOccupancy::hasAltConformers(Residue *masterResidue) const
{
	std::vector<Instance *> instances = _entity->instances();

	for (Instance *instance : instances)
	{
		Residue *local = instance->equivalentLocal(masterResidue);
		AtomGroup *atoms = instance->currentAtoms();

		if (local == nullptr || atoms == nullptr)
		{
			continue;
		}

		for (Atom *a : atoms->atomVector())
		{
			if (a->residueId() == local->id() &&
			    a->conformerPositions().size() > 1)
			{
				return true;
			}
		}
	}

	return false;
}
