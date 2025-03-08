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

#include <gemmi/to_pdb.hpp>

#include "Evolve.h"
#include "Instance.h"
#include "AtomGroup.h"
#include "files/PdbFile.h"
#include "BondSequenceHandler.h"
#include "matrix_functions.h"

Evolve::Evolve(Instance *mol) : _instance(mol)
{
	_instance->load();

	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();
	
	setResidueRange(352, 364);
	setResidueRange(57, 68);
}

Evolve::~Evolve()
{
	_instance->unload();
}

void Evolve::prepareResources()
{
	_resources.allocateMinimum(1);

	/* set up per-bond/atom calculation */
	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
	}

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

}

void Evolve::submitJobAndRetrieve()
{
	submitSingleAxisJob(0, 0, Flag::AtomVector);
	retrieve();
}

glm::vec3 Evolve::reference_axis(int idx)
{
	BondSequenceHandler *sequences = _resources.sequences;
	const std::vector<Parameter *> &params =
	sequences->torsionBasis()->parameters();
	
	Parameter *pStart = params[idx];
	glm::vec3 a = pStart->atom(1)->derivedPosition();
	glm::vec3 b = pStart->atom(2)->derivedPosition();
	return glm::normalize(a - b);
}

// write an Evolve::effects() which returns the Eigen matrix for the motions.
// diverge will SVD and find the ways to vary it without modifying it.
// a stabilise() function will find the ways to return the dot to the ref value.
// diverge() and stabilise() can return vectors which can then be weighted.

Eigen::MatrixXf Evolve::divergence()
{
	glm::vec3 const_ref = reference_axis(_min);
	glm::vec3 moving = reference_axis(_max);
	
	Eigen::MatrixXf matrix(3, _mods.size() - 1);
	int n = 0;

	for (auto it = _mods.begin(); it != _mods.end(); it++)
	{
		int idx = it->first;
		if (idx == _max)
		{
			continue;
		}

		glm::vec3 rot_axis = reference_axis(idx);
		glm::vec3 diff = moving - rot_axis;
		glm::vec3 cross = glm::normalize(glm::cross(rot_axis, diff));
		cross *= glm::length(diff);
		// needs to involve original axis somehow
		matrix(0, n) = cross.x;
		matrix(1, n) = cross.y;
		matrix(2, n) = cross.z;
		n++;
	}
	
	Eigen::JacobiSVD<Eigen::MatrixXf> svd(matrix, Eigen::ComputeFullU | 
	                                      Eigen::ComputeFullV);

	Eigen::MatrixXf v = svd.matrixV();
	return v(Eigen::all, v.cols() - 1) * _correction;
}

Eigen::MatrixXf Evolve::convergence()
{
	BondSequenceHandler *sequences = _resources.sequences;
	const std::vector<Parameter *> &params =
	sequences->torsionBasis()->parameters();

	float dev = 0;
	Eigen::VectorXf targets(_contacts.size());
	targets.setZero();
	int n = 0;
	for (CloseContact &cc : _contacts)
	{
		glm::vec3 v = cc.f->derivedPosition();
		glm::vec3 w = cc.g->derivedPosition();

		float actual = glm::length(v - w);
		float diff = actual - cc.dist;
		dev += diff * diff;
		targets(n) = -diff;
		n++;
	}
	dev /= _contacts.size();
	dev = sqrt(dev);
	std::cout << _num << "\tdeviation: " << dev << std::endl;
	dev = (std::max)(1.f, dev);
	dev *= _stabilisation;
	
	Eigen::MatrixXf cmap(params.size(), _contacts.size());
	cmap.setZero();

	for (int i = 0; i < params.size(); i++)
	{
		if (!params[i]->coversMainChain() || params[i]->isPeptideBond())
		{
			continue;
		}
		
		Atom *rotAtom = params[i]->atom(1);
		
		glm::vec3 x = rotAtom->derivedPosition();
		glm::vec3 y = params[i]->atom(2)->derivedPosition();
		
		int n = 0;
		for (CloseContact &cc : _contacts)
		{
			glm::vec3 v = cc.f->derivedPosition();
			glm::vec3 w = cc.g->derivedPosition();

			float actual = glm::length(v - w);
			float diff = actual - cc.dist;

			int start_to_end = _sep.separationBetween(cc.f, cc.g);
			int start_to_atom = _sep.separationBetween(cc.f, rotAtom);
			int atom_to_end = _sep.separationBetween(rotAtom, cc.g);

			float grad = 0;
			if (atom_to_end + start_to_atom > start_to_end)
			{
				n++;
				continue;
			}
			
			float rot = bond_rotation_on_distance_gradient(x, y, v, w);

			grad = 2 * rot;
			cmap(i, n) = grad;
			n++;
		}
	}

	Eigen::MatrixXf w = cmap.transpose().colPivHouseholderQr().solve(targets);
	w *= _stabilisation;
	return w;
}

void Evolve::evolve()
{
	gemmi::Structure st;
	int i = 0; int j = 0;

	AtomGroup *grp = _instance->currentAtoms();

	while (true)
	{
		Eigen::MatrixXf conv_vec = convergence();
		Eigen::MatrixXf div_vec = divergence();
		_num++;

		int n = 0;
		
		for (int i = 0; i < _angles.size(); i++)
		{
			_angles[i] += conv_vec(i, 0);
		}

		for (auto it = _mods.begin(); it != _mods.end(); it++)
		{
			if (it->first == _max)
			{
				continue;
			}

			float contrib = div_vec(n, 0);
			it->second += contrib;
			n++;
		}

		submitJobAndRetrieve();

		if (j % 10 == 0)
		{
			std::string model_name = std::to_string(i + 1);
			PdbFile::writeAtomsToStructure(grp, st, model_name);
			i++;

			PdbFile::writeStructure(st, "test_evolve.pdb");
		}
		j++;
	}

}

void Evolve::closeContacts()
{
	AtomGroup *grp = _instance->currentAtoms();

	AtomGroup *before = grp->new_subset([this](Atom *const &atom)
	                                    {
		                                   return (atom->isReporterAtom());
	});

	AtomGroup *after = grp->new_subset([this](Atom *const &atom)
	                                   {
		                                  return (atom->isReporterAtom() &&
		                                  atom->residueId() > _maxRes);
	});

	for (Atom *const &f : before->atomVector())
	{
		for (Atom *const &g : before->atomVector())
		{
			if (f == g) continue;
			glm::vec3 v = f->derivedPosition() - g->derivedPosition();
			float dist = glm::length(v);
			if (dist < 6)
			{
				CloseContact cc{f, g, dist};
				std::cout << f->desc() << " " << g->desc() << " " << dist 
				<< std::endl;
				_contacts.push_back(cc);
			}
		}
	}

	delete before;
	delete after;
}

void Evolve::setup()
{
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();

	prepareResources();

	_sep = Separation(_resources.sequences->sequence());

	supplyTorsions(_resources.sequences->manager());
	closeContacts();
	submitJobAndRetrieve();
	
	_worker = new std::thread([this]{ evolve(); });
}

std::set<int> Evolve::findIndices()
{
	Atom *start = _instance->currentAtoms()->atomByIdName(ResidueId(_minRes),
	                                                      "CA");
	Atom *end = _instance->currentAtoms()->atomByIdName(ResidueId(_maxRes),
	                                                    "CA");

	BondSequenceHandler *sequences = _resources.sequences;
	const std::vector<Parameter *> &params =
	sequences->torsionBasis()->parameters();
	
	std::set<int> results;

	int start_to_end = _sep.separationBetween(start, end);
	
	int n = -1;
	for (Parameter *param : params)
	{
		n++;
		if (!param->coversMainChain())
		{
			continue;
		}

		Atom *atom = param->anAtom();
		int start_to_atom = _sep.separationBetween(start, atom);
		int atom_to_end = _sep.separationBetween(atom, end);

		if (atom_to_end + start_to_atom <= start_to_end)
		{
			results.insert(n);
		}
	}

	return results;
}

void Evolve::supplyTorsions(CoordManager *manager)
{
	std::set<int> indices = findIndices();
	if (indices.size() == 0)
	{
		std::cout << "No indices" << std::endl;
		return;
	}

	_min = *indices.begin();
	auto it = indices.end(); it--;
	_max = *(it--);
	std::cout << "min to max: " << _min << " " << _max << std::endl;

	BondSequenceHandler *sequences = _resources.sequences;
	const std::vector<Parameter *> &params =
	sequences->torsionBasis()->parameters();
	
	_angles.resize(params.size());
	
	for (const int &idx : indices)
	{
		_mods[idx] = 0;
	}

	auto grab_torsion = [this]
	(const Coord::Get &get, const int &idx) -> float
	{
		float val = _angles[idx];
		if (idx < _min || idx > _max || _mods.count(idx) == 0)
		{
			return val;
		}

		return val + _mods[idx];
	};

	glm::vec3 const_ref = reference_axis(_min);
	glm::vec3 moving = reference_axis(_max);
	
	manager->setTorsionFetcher(grab_torsion);
}
