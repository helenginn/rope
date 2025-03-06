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

typedef std::vector<std::pair<Atom *, Instance *>> AtomInstanceList;

class AtomCombinator : public std::vector<std::vector<Atom *>>
{
public:
	AtomCombinator(Atom *first, const std::vector<AtomInstanceList> &lists)
	{
		std::vector<int> track(lists.size());
		
		if (lists.size() == 0)
		{
			return;
		}
		
		for (const AtomInstanceList &list : lists)
		{
			if (list.size() == 0)
			{
				return;
			}
		}

		while (true)
		{
			std::vector<Atom *> prep(track.size() + 1);
			prep[0] = first;

			for (int i = 0; i < track.size(); i++)
			{
				int idx = track[i];
				prep[i + 1] = lists[i][idx].first;
			}
			
			push_back(prep);
			
			bool valid = false;

			for (int i = lists.size() - 1; i >= 0; i--)
			{
				if (track[i] < lists[i].size() - 1)
				{
					track[i]++;
					valid = true;
					break;
				}
				else if (i == 0)
				{
					break;
				}
				else
				{
					track[i] = 0;
				}
			}
			
			if (!valid)
			{
				break;
			}
		}
	}
};

Metadata *PolymerEntity::funcBetweenAtoms(const std::vector<FindAtom> &ps,
                                          const std::string &header,
                                          const Calculate &calculate,
                                          const Compare &compare)
{
	Metadata *md = new Metadata();
	
	for (Model *model : _models)
	{
		model->load(Model::NoGeometry);

		AtomInstanceList matching = ps[0](model, nullptr);

		for (auto match : matching)
		{
			std::vector<AtomInstanceList> more_matches;

			for (int i = 1; i < ps.size(); i++)
			{
				AtomInstanceList next = ps[i](model, match.second);
				more_matches.push_back(next);
			}

			AtomCombinator combinator(match.first, more_matches);
			
			if (combinator.size() == 0)
			{
				continue;
			}
			
			std::vector<float> results;
			for (std::vector<Atom *> atoms : combinator)
			{
				float candidate = calculate(atoms);
				results.push_back(candidate);
			}
			
			float best = compare(results);

			Metadata::KeyValues kv;

			kv["instance"] = Value(match.second->id());
			kv[header] = Value(f_to_str(best, 2));
			md->addKeyValues(kv, true);
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
	regionManager().setEntity(this);
}

TorsionData PolymerEntity::prepareTorsionGroup()
{
	std::vector<ResidueTorsion> headers;
	_sequence.addResidueTorsions(headers);
	size_t num = headers.size();

	TorsionData group(num);
	group.addHeaders(headers);

	return group;
}

PositionData PolymerEntity::preparePositionGroup()
{
	std::vector<Atom3DPosition> headers;
	_sequence.addAtomPositionHeaders(headers);
	
	PositionData group(headers.size());
	group.addHeaders(headers);

	return group;
}

BFactorData PolymerEntity::prepareBFactorGroup()
{
	std::vector<Atom3DPosition> headers;
	_sequence.addAtomPositionHeaders(headers);
	
	BFactorData group(headers.size());
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
