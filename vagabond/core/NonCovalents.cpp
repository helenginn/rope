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

#include "NonCovalents.h"
#include "BondSequence.h"
#include "Instance.h"
#include "Atom.h"
#include "engine/Task.h"

using Eigen::MatrixXf;

NonCovalents::NonCovalents()
{

}

template <typename Func>
int atom_index_for_condition(BondSequence *const &seq, const Func &acceptable)
{
	int i = 0;
	for (const AtomBlock &block : seq->blocks())
	{
		if (acceptable(block.atom))
		{
			return i;
		}
		i++;
	}

	return -1;
}

int atom_index_for_desc(BondSequence *const &seq, const std::string &desc)
{
	return atom_index_for_condition(seq, [desc](Atom *const &atom)
	                                {
		                               return atom && atom->desc() == desc;
		                            });
}

void NonCovalents::findBondedAtoms(BondSequence *const &seq, Bond &bond)
{
	bond.left_lookup = atom_index_for_desc(seq, bond.left);
	bond.right_lookup = atom_index_for_desc(seq, bond.right);
}

void NonCovalents::findMiddleAtoms(BondSequence *const &seq, Bond &bond)
{
	if (bond.left_lookup < 0 && bond.right_lookup < 0)
	{
		return;
	}

	Atom *left_atom = seq->blocks()[bond.left_lookup].atom;
	Atom *right_atom = seq->blocks()[bond.right_lookup].atom;
	glm::vec3 left = left_atom->initialPosition();
	glm::vec3 right = right_atom->initialPosition();

	glm::vec3 middle = (left + right) / 2.f;
	
	bond.middle_lookup = atom_index_for_condition(seq, 
	                                              [middle](Atom *const &atom)
	{
		return atom && glm::length(atom->initialPosition() - middle) < 0.8;
	});
	
	if (bond.middle_lookup < 0)
	{
		std::cout << "Could not resolve hydrogen bond" << std::endl;
		return;
	}

	Atom *h = seq->blocks()[bond.middle_lookup].atom;
	middle = h->initialPosition();

	if (h->isConnectedToAtom(left_atom))
	{
		bond.left_is_donor = true;
		bond.left_contributor = [&bond](BondSequence *seq) -> glm::vec3
		{
			glm::vec3 left = seq->blocks()[bond.left_lookup].my_position();
			glm::vec3 middle = seq->blocks()[bond.middle_lookup].my_position();
			return middle + (bond.frac - 1.f) * left;
		};
		bond.right_contributor = [&bond](BondSequence *seq) -> glm::vec3
		{
			glm::vec3 right = seq->blocks()[bond.right_lookup].my_position();
			return bond.frac * right;
		};
	}
	else
	{
		bond.left_contributor = [&bond](BondSequence *seq) -> glm::vec3
		{
			glm::vec3 left = seq->blocks()[bond.left_lookup].my_position();
			return (1 - bond.frac) * left;
		};
		bond.right_contributor = [&bond](BondSequence *seq) -> glm::vec3
		{
			glm::vec3 middle = seq->blocks()[bond.middle_lookup].my_position();
			glm::vec3 right = seq->blocks()[bond.right_lookup].my_position();
			return middle - bond.frac * right;
		};
		
		glm::vec3 tmp = right;
		right = left;
		left = tmp;
	}

	bond.frac = glm::length(middle - left) / glm::length(right - left);
	std::cout << " frac: " << bond.frac << std::endl;
	
}

void NonCovalents::assignInstancesToAtoms(BondSequence *const &seq)
{
	int i = 0;
	std::cout << "instances: " << _instances.size() << std::endl;
	for (AtomBlock &block : seq->blocks())
	{
		if (block.atom)
		{
			for (Instance *instance : _instances)
			{
				if (instance->atomBelongsToInstance(block.atom))
				{
					_atomNumbers[instance].push_back(i);
				}
			}
		}

		i++;
	}
}

void NonCovalents::prepare(BondSequence *const &seq)
{
	for (Bond &bond : _bonds)
	{
		findBondedAtoms(seq, bond);
		findMiddleAtoms(seq, bond);
		_map[bond.left_instance][bond.right_instance] = &bond;
	}
	
	std::vector<Bond> filtered;
	for (Bond &bond : _bonds)
	{
		if (bond.middle_lookup >= 0)
		{
			filtered.push_back(bond);
		}
	}

	_bonds = filtered;
	
	assignInstancesToAtoms(seq);
	
	if (_instances.size() == 2)
	{
		prepareOne();
	}
	else
	{
		prepareMatrix();
	}
}

void NonCovalents::prepareOne()
{
	int data_number = _bonds.size();
	_leftMatrix = MatrixXf(3, data_number);
	_rightMatrix = MatrixXf(3, data_number);

	int i = 0;
	for (Bond &bond : _bonds)
	{
		bond.left_drop[0] = 0;
		bond.left_drop[1] = i;
		bond.right_drop[0] = 0;
		bond.right_drop[1] = i;
		i++;
	}
}

void NonCovalents::prepareMatrix()
{
	int groups = _instances.size() * 3;
	int data_number = _bonds.size();

	_leftMatrix = MatrixXf(groups, data_number);
	_rightMatrix = MatrixXf(groups, data_number);
	
	for (auto it = _map.begin(); it != _map.end(); it++)
	{
		std::map<Instance *, Bond *> &inner = it->second;
		for (auto jt = inner.begin(); jt != inner.end(); jt++)
		{

		}
	}
}

std::function<BondSequence *(BondSequence *)> NonCovalents::align()
{
	return [this](BondSequence *seq) -> BondSequence *
	{
		// get local copies of matrix templates
		Eigen::MatrixXf l = _leftMatrix;
		Eigen::MatrixXf r = _rightMatrix;
		if (l.rows() == 0) return seq;

		for (const Bond &bond : _bonds)
		{
			glm::vec3 left_contrib = bond.left_contributor(seq);
			glm::vec3 right_contrib = bond.right_contributor(seq);
			
			auto drop_bond = [](Eigen::MatrixXf &mat, const int *drop, 
			                    const glm::vec3 &contrib)
			{
				int x = drop[0]; int y = drop[1];
				for (int i = 0; i < 3; i++)
				{
					float &f = mat(x + i, y);
					f = 2 * contrib[i];
				}
			};
			
			drop_bond(l, &bond.left_drop[0], left_contrib);
			drop_bond(r, &bond.right_drop[0], right_contrib);
		}
		
		l.transposeInPlace();
		r.transposeInPlace();
		
		std::cout << "===================" << std::endl;
		std::cout << "difference: " << std::endl << r - l << std::endl;
		std::cout << std::endl;
		
		auto get_ave_trans = [](const Eigen::MatrixXf &mat) -> Eigen::Vector3f
		{
			Eigen::Vector3f diff;
			for (int i = 0; i < mat.rows(); i++)
			{
				diff += mat(i, {0, 1, 2}).transpose();
			}

			diff /= (float)(mat.rows());
			return diff;
		};

		Eigen::Vector3f overall = get_ave_trans(l);
		Eigen::Vector3f first_diff = get_ave_trans(r - l);
		std::cout << "First diff: " << first_diff << std::endl;
		for (int i = 0; i < l.rows(); i++)
		{
			l(i, {0, 1, 2}) -= overall.transpose();
			r(i, {0, 1, 2}) -= (overall - first_diff).transpose();
		}
		std::cout << "-> new left: " << std::endl;
		std::cout << l << std::endl;
		std::cout << "-> new right: " << std::endl;
		std::cout << r << std::endl;
		
		Eigen::MatrixXf sol = l.colPivHouseholderQr().solve(r);
		
		if (sol.rows() != 3 || sol.cols() != 3)
		{
			return seq;
		}

		Eigen::JacobiSVD<MatrixXf> svd(sol, Eigen::ComputeFullU | Eigen::ComputeFullV);

		Eigen::MatrixXf u = svd.matrixU();
		Eigen::MatrixXf v = svd.matrixV();
		Eigen::MatrixXf fixed = u * v.transpose();
		sol({0, 1, 2}, {0, 1, 2}) = fixed;

		Eigen::MatrixXf translate = MatrixXf::Identity(4, 4);
		Eigen::MatrixXf back = MatrixXf::Identity(4, 4);
		Eigen::MatrixXf rotate = MatrixXf::Identity(4, 4);

		translate(3, {0, 1, 2}) += overall.transpose();
		back(3, {0, 1, 2}) -= overall.transpose();
		rotate({0, 1, 2}, {0, 1, 2}) = sol;
		
		Eigen::MatrixXf there_and_back = back * rotate * translate;

		mat4x4 transform = mat4x4(1.f);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				transform[i][j] = there_and_back(i, j);
				if (transform[i][j] != transform[i][j])
				{
					return seq;
				}
			}
		}

		std::cout << "Transform: " << std::endl;
		std::cout << transform << std::endl;

		for (Instance *inst : _instances)
		{
			if (inst == _instances.back())
			{
				continue;
			}

			std::vector<int> &idxs = _atomNumbers[inst];

			for (const int &idx : idxs)
			{
				mat4x4 &basis = seq->blocks()[idx].basis;
				vec4 tmp = basis[3]; tmp[3] = 1.;
				basis[3] = transform * tmp;
			}
		}
		
		return seq;
	};
}

Task<BondSequence *, BondSequence *> *NonCovalents::align_task()
{
	auto alignment = align();
	auto *task = new Task<BondSequence *, BondSequence *>(alignment);
	return task;
}
