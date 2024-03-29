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

#ifndef __vagabond__AtomRecall__
#define __vagabond__AtomRecall__

#include "Atom.h"
#include "Model.h"
#include "Instance.h"
#include "Atom3DPosition.h"

inline auto force_other_instance()
{
	return [](Instance *reference, Instance *search) -> bool
	{
		return reference != search;
	};
}

inline auto force_same_instance()
{
	return [](Instance *reference, Instance *search) -> bool
	{
		return reference == search;
	};
}

inline auto instance_has_entity(Entity *entity)
{
	return [entity](Instance *search) -> bool
	{
		return search->entity() == entity;
	};
}

template <typename CheckInstance>
inline auto get_instance(const CheckInstance &check)
{
	return [check](Model *model, Instance *reference) -> std::vector<Instance *>
	{
		std::vector<Instance *> instances = model->instances();
		std::vector<Instance *> ret;
		for (Instance *inst : instances)
		{
			if (check(inst, reference))
			{
				ret.push_back(inst);
			}
		}

		return ret;
	};
}

template <typename CheckInstance>
inline auto search_models(const CheckInstance &check, const Atom3DPosition &pos)
{
	auto get_from = get_instance(check);

	return [get_from, pos](Model *model, Instance *reference) 
	-> std::vector<std::pair<Atom *, Instance *>>
	{
		std::vector<Instance *> instances = get_from(model, reference);
		std::vector<std::pair<Atom *, Instance *>> atoms;

		for (Instance *inst : instances)
		{
			Atom *atom = inst->atomForIdentifier(pos);
			if (atom)
			{
				std::pair<Atom *, Instance *> pair(atom, inst);
				atoms.push_back(pair);
			}

		}
		return atoms;
	};
}

typedef std::function<std::vector<std::pair<Atom *, Instance *>>
(Model *model, Instance *reference)> 
FindAtom;


inline FindAtom search_models_any_instance(Entity *entity, const Atom3DPosition &pos)
{
	auto check = [entity](Instance *instance, Instance *reference) -> bool
	{
		return instance_has_entity(entity)(instance);
	};

	return search_models(check, pos);
}

inline FindAtom search_models_different_instance(Entity *entity, 
                                             const Atom3DPosition &pos)
{
	auto check = [entity](Instance *instance,
	                               Instance *reference) -> bool
	{
		return instance_has_entity(entity)(instance)
				&& force_other_instance()(instance, reference);
	};

	return search_models(check, pos);
}

inline FindAtom search_models_same_instance(Entity *entity, const Atom3DPosition &pos)
{
	auto check = [entity](Instance *instance, Instance *reference) -> bool
	{
		return instance_has_entity(entity)(instance)
				&& force_same_instance()(instance, reference);
	};

	return search_models(check, pos);
}

inline auto closest_to(float target)
{
	return [target](const std::vector<float> &values)
	{
		float diff = FLT_MAX;
		float best = FLT_MAX;

		for (const float &f : values)
		{
			float d = fabs(f - target);
			if (d < diff)
			{
				best = f;
				diff = d;
			}
		}

		return best;
	};
}

inline float distance_between(const std::vector<Atom *> &atoms)
{
	glm::vec3 first = atoms[0]->initialPosition();
	glm::vec3 last = atoms[1]->initialPosition();

	float dist = glm::length(first - last);
	return dist;
}

inline float angle_between(const std::vector<Atom *> &atoms)
{
	glm::vec3 middle = atoms[1]->initialPosition();
	glm::vec3 first = atoms[0]->initialPosition() - middle;
	glm::vec3 last = atoms[2]->initialPosition() - middle;
	first = glm::normalize(first);
	last = glm::normalize(last);

	float angle = rad2deg(glm::angle(first, last));
	return angle;
}

#endif
