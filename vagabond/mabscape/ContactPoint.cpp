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
#include <vagabond/utils/FileReader.h>
#include <algorithm>
#include <cmath>
#include <cfloat>

// an orientation which superposes worse than this (Angstroms) is flagged

ContactPoint::ContactPoint(Fiducial &fiducial, Antigens &antigens,
                           bool find_instances)
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

	if (!find_instances)
	{
		return;
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

			float rmsd = 0;
			glm::mat4x4 new_mat = 
			Instance::superposeInstances(ltrunc, arrangement, false, &rmsd);

			bool added = add_if_new(new_mat);

			if (added)
			{
				_entries.push_back({new_mat, ltrunc, arrangement});
				_orientations.push_back({new_mat, rmsd});
				std::cout << "RMSD of orientation " << _entries.size() 
				<< ": " << rmsd << " A" << std::endl;

				// a poor alignment, or none at all (not a number), means
				// this orientation cannot be trusted
				if (rmsd != rmsd || rmsd > MAX_RMSD)
				{
					std::string msg = "Orientation " + 
					std::to_string(_entries.size()) + " of antibody " + 
					_fiducial.name + " aligns with RMSD " + 
					(rmsd != rmsd ? std::string("unknown") : 
					 f_to_str(rmsd, 1) + " A");
					_warnings.push_back({msg, new_mat, rmsd});
				}
			}
		}
	}

	_fiducial.model.unload();
	_chosen->model.unload();
	
	establishMidpoint();
	recalibrateToFirst();
}

// atoms which cannot stand for a position: coordinates which are not numbers
// (or are huge), unplaced ones left at the origin, and the reporter atoms
// which only help the ribbons
static bool usableAtom(Atom *atom)
{
	const glm::vec3 &p = atom->derivedPosition();
	if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
	{
		return false;
	}

	if (glm::length(p) < 1e-3f || glm::length(p) > 1e5f)
	{
		return false;
	}

	return !atom->isReporterAtom();
}

// The "median" atom of a group: the one with the smallest total distance to
// all the others (a medoid), among the usable atoms. Unlike the atom nearest
// the mean position, this is in the dense middle of the group, as the total
// distance is not squared and so a distant atom cannot drag it away.
static Atom *nearestToCentre(AtomGroup *group)
{
	std::vector<Atom *> atoms;
	std::vector<glm::vec3> pos;
	for (Atom *atom : group->atomVector())
	{
		if (usableAtom(atom))
		{
			atoms.push_back(atom);
			pos.push_back(atom->derivedPosition());
		}
	}

	Atom *best = nullptr;
	float bestTotal = FLT_MAX;
	for (size_t i = 0; i < atoms.size(); i++)
	{
		float total = 0;
		for (size_t j = 0; j < atoms.size(); j++)
		{
			total += glm::length(pos[i] - pos[j]);
		}

		if (total < bestTotal)
		{
			bestTotal = total;
			best = atoms[i];
		}
	}

	return best;
}

// separate patches of atoms: linked through neighbours within {link}
// Angstroms. Fills in the patch of each atom and returns how many there are.
static int findPatches(const std::vector<Atom *> &atoms, 
                       std::vector<int> &patch, float link = 6.f)
{
	patch.assign(atoms.size(), -1);
	int count = 0;

	for (size_t i = 0; i < atoms.size(); i++)
	{
		if (patch[i] >= 0) continue;

		std::vector<size_t> stack = {i};
		patch[i] = count;
		while (stack.size())
		{
			size_t at = stack.back(); stack.pop_back();
			glm::vec3 here = atoms[at]->derivedPosition();
			for (size_t j = 0; j < atoms.size(); j++)
			{
				if (patch[j] < 0 && 
				    glm::length(here - atoms[j]->derivedPosition()) < link)
				{
					patch[j] = count;
					stack.push_back(j);
				}
			}
		}
		count++;
	}

	return count;
}

// A model can hold the interface more than once (an antibody on each of
// several antigen copies, say), which would put the centre of the "interface"
// somewhere between them. So just one is taken: the antibody atoms within
// {reach} of the antigen and the antigen atoms within {reach} of the antibody
// are each split into separate patches, every antibody patch is paired with
// the antigen patch it touches most closely, and the pair with the most atoms
// is the interface.
void ContactPoint::interfaceAtoms(AtomGroup *antibody, AtomGroup *antigen,
                                  AtomGroup *&antibodyFace, 
                                  AtomGroup *&antigenFace, float reach)
{
	auto position = [](Atom *const &a) { return a->derivedPosition(); };
	GroupBounds antibodyBounds(antibody, position);
	GroupBounds antigenBounds(antigen, position);

	AtomGroup *allAntibody = 
	antigenBounds.atoms_from_other_group_within(antibody, reach);
	AtomGroup *allAntigen = 
	antibodyBounds.atoms_from_other_group_within(antigen, reach);

	// only atoms which can stand for a position
	std::vector<Atom *> left, right;
	for (Atom *atom : allAntibody->atomVector())
	{
		if (usableAtom(atom)) left.push_back(atom);
	}
	for (Atom *atom : allAntigen->atomVector())
	{
		if (usableAtom(atom)) right.push_back(atom);
	}
	delete allAntibody;
	delete allAntigen;

	antibodyFace = new AtomGroup();
	antigenFace = new AtomGroup();

	std::vector<int> leftPatch, rightPatch;
	int leftCount = findPatches(left, leftPatch);
	int rightCount = findPatches(right, rightPatch);
	if (leftCount == 0 || rightCount == 0)
	{
		return;
	}

	std::vector<int> leftSize(leftCount, 0), rightSize(rightCount, 0);
	for (int id : leftPatch) leftSize[id]++;
	for (int id : rightPatch) rightSize[id]++;

	// the closest approach between each antibody patch and antigen patch
	std::vector<float> closest(leftCount * rightCount, FLT_MAX);
	for (size_t i = 0; i < left.size(); i++)
	{
		glm::vec3 p = left[i]->derivedPosition();
		for (size_t j = 0; j < right.size(); j++)
		{
			float d = glm::length(p - right[j]->derivedPosition());
			float &best = closest[leftPatch[i] * rightCount + rightPatch[j]];
			best = std::min(best, d);
		}
	}

	int bestLeft = -1, bestRight = -1, bestSize = -1;
	for (int l = 0; l < leftCount; l++)
	{
		int partner = 0;
		for (int r = 1; r < rightCount; r++)
		{
			if (closest[l * rightCount + r] < closest[l * rightCount + partner])
			{
				partner = r;
			}
		}

		int size = leftSize[l] + rightSize[partner];
		if (size > bestSize)
		{
			bestSize = size;
			bestLeft = l;
			bestRight = partner;
		}
	}

	for (size_t i = 0; i < left.size(); i++)
	{
		if (leftPatch[i] == bestLeft) antibodyFace->add(left[i]);
	}
	for (size_t j = 0; j < right.size(); j++)
	{
		if (rightPatch[j] == bestRight) antigenFace->add(right[j]);
	}
}

bool ContactPoint::interfaceMidpoint(AtomGroup *antibody, AtomGroup *antigen,
                                     glm::vec3 &midpoint, float reach)
{
	AtomGroup *antibodyFace = nullptr;
	AtomGroup *antigenFace = nullptr;
	interfaceAtoms(antibody, antigen, antibodyFace, antigenFace, reach);
	
	Atom *left = nearestToCentre(antibodyFace);
	Atom *right = nearestToCentre(antigenFace);

	bool found = (left && right);
	if (found)
	{
		midpoint = (left->derivedPosition() + right->derivedPosition()) / 2.f;
	}

	delete antibodyFace;
	delete antigenFace;
	return found;
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
	
	std::cout << "Number of antibody atoms: " << abs->size() << std::endl;
	std::cout << "Number of antigen atoms: " << anti->size() << std::endl;

	if (!interfaceMidpoint(abs, anti, _reference))
	{
		throw std::runtime_error("No interface atoms for antibody");
	}
	
	std::cout << "Reference: " << _reference << std::endl;
	
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

static json matrix_to_json(const glm::mat4x4 &mat)
{
	json j = json::array();
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			j.push_back(mat[col][row]);
		}
	}

	return j;
}

static glm::mat4x4 matrix_from_json(const json &j)
{
	glm::mat4x4 mat(1.f);
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			mat[col][row] = j.at(col * 4 + row).get<float>();
		}
	}

	return mat;
}

json ContactPoint::inputs() const
{
	json j;
	// bump whenever the way the midpoint is worked out changes
	j["midpoint_method"] = 5;
	j["antigen"] = _fiducial.antigen;
	j["fiducial_file"] = _fiducial.model.filename();
	j["fiducial_time"] = (long long)file_mod_time(_fiducial.model.filename());
	j["antigen_file"] = _chosen->model.filename();
	j["antigen_time"] = (long long)file_mod_time(_chosen->model.filename());

	std::vector<std::string> fid_ents, antigen_ents;
	for (const std::string &entity : _fiducial.entities)
	{
		fid_ents.push_back(entity);
	}
	for (const std::string &entity : _chosen->entities)
	{
		antigen_ents.push_back(entity);
	}

	std::sort(fid_ents.begin(), fid_ents.end());
	std::sort(antigen_ents.begin(), antigen_ents.end());
	j["fiducial_entities"] = fid_ents;
	j["antigen_entities"] = antigen_ents;
	return j;
}

json ContactPoint::alignmentJson() const
{
	json j;
	j["inputs"] = inputs();
	j["reference"] = {_reference.x, _reference.y, _reference.z};

	json transforms = json::array();
	for (const glm::mat4x4 &transform : _transforms)
	{
		transforms.push_back(matrix_to_json(transform));
	}
	j["transforms"] = transforms;

	json warnings = json::array();
	for (const Warning &warning : _warnings)
	{
		json w;
		w["text"] = warning.text;
		w["transform"] = matrix_to_json(warning.transform);
		w["rmsd"] = warning.rmsd;
		warnings.push_back(w);
	}
	j["warnings"] = warnings;

	json orientations = json::array();
	for (const Orientation &orientation : _orientations)
	{
		json o;
		o["transform"] = matrix_to_json(orientation.transform);
		o["rmsd"] = orientation.rmsd;
		orientations.push_back(o);
	}
	j["orientations"] = orientations;

	return j;
}

bool ContactPoint::restore(const json &saved)
{
	// saves from before every orientation was kept are worked out again
	if (!saved.is_object() || !saved.count("inputs") || 
	    !saved.count("orientations") || saved.at("inputs") != inputs())
	{
		return false;
	}

	try
	{
		std::vector<glm::mat4x4> transforms;
		for (const json &t : saved.at("transforms"))
		{
			transforms.push_back(matrix_from_json(t));
		}

		if (transforms.empty())
		{
			return false;
		}

		std::vector<Warning> warnings;
		for (const json &w : saved.at("warnings"))
		{
			warnings.push_back({w.at("text"), matrix_from_json(w.at("transform")),
			                    w.at("rmsd")});
		}

		std::vector<Orientation> orientations;
		for (const json &o : saved.at("orientations"))
		{
			orientations.push_back({matrix_from_json(o.at("transform")),
			                        o.at("rmsd")});
		}

		const json &ref = saved.at("reference");
		_reference = glm::vec3(ref.at(0).get<float>(), ref.at(1).get<float>(),
		                       ref.at(2).get<float>());
		_transforms = transforms;
		_warnings = warnings;
		_orientations = orientations;
		_applied = glm::mat4{1.f};
	}
	catch (const json::exception &err)
	{
		return false;
	}

	return true;
}
