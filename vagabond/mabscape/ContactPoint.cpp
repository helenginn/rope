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
#include "Mab.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/GroupBounds.h>
#include <algorithm>

ContactPoint::ContactPoint(Fiducial &fiducial, Antigens &antigens)
: _fiducial(fiducial), _antigens(antigens)
{
	_transforms.clear();

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

void ContactPoint::findMapping()
{
	if (!_chosen)
	{
		throw std::runtime_error("Could not find chosen antigen");
	}

	std::cout << std::endl;
	_fiducial.model.load();
	_chosen->model.load();
	// need to get a mapping of every transformation matrix to every other
	
	std::map<Instance *, std::map<Instance *, glm::mat4x4>> fid_to_agn;
	std::map<std::string, OpSet<Instance *>> lefts;
	std::map<std::string, OpSet<Instance *>> rights;

	for (Instance *left : _iFidAntigens)
	{
		for (Instance *right : _iAntigens)
		{
			if (left->entity_id() != right->entity_id())
			{
				continue;
			}
			
			lefts[left->entity_id()] += left;
			rights[right->entity_id()] += right;

			glm::mat4x4 result = left->superposeOn(right, false);
			fid_to_agn[left][right] = result;
		}
	}

	auto write_instances = []<class Container>(const Container &c)
	{
		for (Instance *const &inst : c)
		{
			if (inst == nullptr) std::cout << "nullptr ";
			else std::cout << inst->id() << " ";
		}
		std::cout << std::endl;
	};
	
	std::cout << "Lefts (fid): ";
	for (auto &pair : lefts)
	{
		std::cout << pair.first << ": ";
		write_instances(pair.second);
	}

	std::cout << "Rights (ant): ";
	for (auto &pair : rights)
	{
		std::cout << pair.first << ": ";
		write_instances(pair.second);
	}
	
	auto check_permutation = [&fid_to_agn, &write_instances, this]
	(const std::vector<Instance *> &ref, std::list<Instance *> &arranged)
	{
		std::vector<glm::mat4x4> mats;
		std::cout << "Arrangement: ";
		write_instances(arranged);

		auto it = ref.begin(); auto jt = arranged.begin();
		
		for (; it != ref.end(); it++, jt++)
		{
			Instance *left = *it; Instance *right = *jt;
			if (left == nullptr)
			{
				continue;
			}
			mats.push_back(fid_to_agn[left][right]);
		}

		float ave = 0;
		float count = 0;
		for (int i = 0; i < mats.size() - 1; i++)
		{
			const glm::mat4x4 &left = mats[i];
			for (int j = i + 1; j < mats.size(); j++)
			{
				const glm::mat4x4 &right = mats[j];
				ave += similarity_score(left, right);
				count++;
			}
		}
		ave /= count;
		if (ave != ave) { ave = 0; }
		std::cout << "score: " << ave << std::endl;
		return ave;
	};
	
	for (const auto &pair : lefts)
	{
		const std::string &entity = pair.first;
		std::cout << "Doing entity " << entity << std::endl;
		std::vector<Instance *> leftInsts 
		= {pair.second.begin(), pair.second.end()};
		std::list<Instance *> leftList 
		= {pair.second.begin(), pair.second.end()};
		
		std::cout << "Rights size: " << rights.at(entity).size() << std::endl;

		if (rights.count(entity) == 0 ||
		    rights.at(entity).size() < leftInsts.size())
		{
			throw std::runtime_error("More instances of " + entity + " in "\
			                         "antibody than in antigen; cannot align");
		}

		const OpSet<Instance *> &rightInsts = rights[entity];
		int orig_size = leftInsts.size();
		std::cout << "Sizes: " << orig_size << " to " << 
		rightInsts.size() << std::endl;
		// fill any other positions with nulls
		leftInsts.resize(rightInsts.size());

		std::list<Instance *> arranged = 
		std::list<Instance *>(rightInsts.begin(), rightInsts.end());
		write_instances(leftInsts);
		float best_score = FLT_MAX;
		std::list<Instance *> arrangement;
		OpSet<std::list<Instance *>> arrangements;

		std::cout << "with: " << leftInsts.size() << " left instances" << std::endl;
		do
		{
			float score = check_permutation(leftInsts, arranged);
			std::list<Instance *> trunc = arranged;
			trunc.resize(orig_size);

			if (score < best_score) 
			{
				write_instances(trunc);
				best_score = score;
				std::cout << " -> " << score << std::endl;
				arrangement = trunc;
			}
			if (score < _threshold)
			{
				arrangements.insert(trunc);
			}
		}
		while (std::next_permutation(arranged.begin(), arranged.end()));
		
		std::cout << "Collected " << arrangements.size() << " arrangements" << std::endl;
		std::cout << "... -> best: " << std::endl;
		write_instances(arrangement);
		if (arrangements.size() == 0)
		{
			arrangements.insert(arrangement);
		}
		
		for (auto arrangement : arrangements)
		{
			std::list<Instance *> ltrunc = leftList;
			ltrunc.resize(arrangement.size());

			glm::mat4x4 new_mat = 
			Instance::superposeInstances(ltrunc, arrangement, false);

			bool added = add_if_new(new_mat);

			if (added)
			{
				_entries.push_back({new_mat, ltrunc, arrangement});
			}
		}
	}

	_fiducial.model.unload();
	_chosen->model.unload();
	
	establishMidpoint();
	recalibrateToFirst();
}

void ContactPoint::establishMidpoint()
{
	_fiducial.model.load();
	applyTransform(_transforms.front());

	AtomGroup *abs = new AtomGroup();
	AtomGroup *anti = new AtomGroup();
	
	for (Instance *const &inst : _iFiducials)
	{
		abs->add(inst->currentAtoms());
	}

	for (Instance *const &inst : _iFidAntigens)
	{
		anti->add(inst->currentAtoms());
	}
	
	GroupBounds bounds(abs, [](Atom *a) { return a->derivedPosition(); });
	AtomGroup *interface = bounds.atoms_from_other_group_within(anti, 5);
	std::cout << "Number of antibody atoms: " << abs->size() << std::endl;
	std::cout << "Number of antigen atoms: " << anti->size() << std::endl;
	std::cout << "Number of shared atoms: " << interface->size() << std::endl;
	
	if (interface->size() == 0)
	{
		throw std::runtime_error("No interface atoms for antibody");
	}
	
	Atom *central = interface->mostCentralAtom
	([](Atom *a) { return a->derivedPosition();});
	std::cout << "Central atom: " << central->desc() << std::endl;
	_reference = central->derivedPosition();
	std::cout << "Reference: " << _reference << std::endl;
	
	delete interface;
	delete abs;
	delete anti;

	_fiducial.model.unload();
}

void ContactPoint::applyTransform(const glm::mat4x4 &which)
{
	AtomGroup *myAtoms = _fiducial.model.currentAtoms();
	
	struct loop_over_atoms
	{
		loop_over_atoms(AtomGroup *atoms) : _atoms(atoms) {};

		glm::vec3 *operator()()
		{
			if (n >  0)
			{
				glm::vec3 d = _atoms->atomVector()[n - 1]->derivedPosition();
				_atoms->atomVector()[n - 1]->setDerivedPosition(d);
			}

			if (n >= _atoms->atomVector().size())
			{
				return nullptr;
			}

			glm::vec3 &v = _atoms->atomVector()[n]->derivedPosition();
			n++;
			return &v;
		};

		int n = 0;
		AtomGroup *_atoms{};
	};

	Symmetry::applyTransform(which, loop_over_atoms(myAtoms));
}
