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
#include "GroupBounds.h"
#include "AtomGroup.h"
#include "HelpKnot.h"
#include "Instance.h"
#include "Atom.h"
#include "engine/Task.h"
#include <fstream>
#include <algorithm>

using Eigen::MatrixXf;

NonCovalents::NonCovalents()
{

}

void write_to_file(const Eigen::MatrixXf &src, std::string file)
{
	std::ofstream f;
	f.open(file);
	for (int j = 0; j < src.cols(); j++)
	{
		for (int i = 0; i < src.rows(); i++)
		{
			f << src(i, j) << (i < src.rows() - 1 ? " " : "");
		}
		f << std::endl;
	}
	f.close();
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

int atom_index_for_atom(BondSequence *const &seq, Atom *const &atom)
{
	return atom_index_for_condition(seq, [atom](Atom *const &other)
	                                {
		                               return atom == other;
		                            });
}

bool atomBelongsToSegment(Atom *atom, Segment &seg)
{
	return (seg.grp->hasAtom(atom));
}

OpSet<Atom *> total_invariant_atoms(const std::vector<NonCovalents::Interface> 
                                    &faces, const Segment &invariant)
{
	int total = 0;
	OpSet<Atom *> all;
	for (const NonCovalents::Interface &face : faces)
	{
		if (face.left == invariant)
		{
			all += face.lefts.atoms;
		}
		if (face.right == invariant)
		{
			all += face.rights.atoms;
		}
	}
	return all;
}


void NonCovalents::assignSegmentsToAtoms(BondSequence *const &seq)
{
	Segment *segment = nullptr;
	std::vector<int> numbers;
	int n = 0;
	int m = 0;
	int i = 0;
	
	auto add_segment = [this, &numbers, &n, &m](Segment *&segment)
	{
		if (segment && segment->grp->size() > 0)
		{
			std::cout << "Segment " << n << ": " << segment->grp->size() << std::endl;
			_atomNumbers[*segment] = numbers;
			_segments.push_back(*segment);
			numbers.clear();

			if (_invariant.num < 0)
			{
				_invariant = *segment;
				std::cout << "Assigning invariant" << std::endl;
			}
			_segment2Idx[*segment] = m;
			m++;

			delete segment;
			segment = nullptr;
		}

	};

	for (AtomBlock &block : seq->blocks())
	{
		if (!block.atom)
		{
			add_segment(segment);
			segment = new Segment(n);
			n++;
		}

		if (block.atom)
		{
			numbers.push_back(i);
			segment->grp->add(block.atom);
			_atom2Seq[block.atom] = i;
		}

		i++;
	}
	
	if (segment && segment->grp->size() > 0)
	{
		add_segment(segment);
	}
}

NonCovalents::Interface NonCovalents::findInterface(Segment first, 
                                                    Segment second)
{
	Interface interface;
	interface.left = first;
	interface.right = second;

	AtomGroup *grp = first.grp;
	AtomGroup *compare = second.grp;
	
	GroupBounds bounds_grp(grp);
	GroupBounds bounds_compare(compare);

	for (int i = 0; i < 3; i++)
	{
		glm::vec3 &min1 = bounds_grp.min;
		glm::vec3 &max1 = bounds_compare.max;
		if (max1[i] < min1[i] - 5) return interface;

		glm::vec3 &max2 = bounds_grp.max;
		glm::vec3 &min2 = bounds_compare.min;
		if (max2[i] < min2[i] - 5) return interface;
	}
	
	auto valid_distance = [](glm::vec3 &a, glm::vec3 &b) -> bool
	{
		const float max = 4.5f;
		// bool bad = false;
		for (int i = 0; i < 3; i++)
		{
			if (fabs(a[i] - b[i]) > max)
			{
				return false;
			}
		}

		if (glm::dot(a - b, a - b) > max * max)
		{
			return false;
		}

		return true;
	};

	for (Atom *atom : grp->atomVector())
	{
		if (atom->elementSymbol() == "H")
		{
			continue;
		}
		glm::vec3 p1 = atom->otherPosition("target");
		for (Atom *other : compare->atomVector())
		{
			glm::vec3 p2 = other->otherPosition("target");
			if (other->elementSymbol() == "H")
			{
				continue;
			}
			if (!valid_distance(p1, p2))
			{
				continue;
			}

			interface.atom_pairs.push_back({atom, other});
			interface.lefts.atoms.insert(atom);
			interface.rights.atoms.insert(other);
		}
	}
	
	return interface;
}

auto closest_atoms(OpSet<Atom *> &atoms, float bond_len)
// Add total ~ 9 fiducials. 9C4 = 126.
{
	return [&atoms, bond_len](Atom *const &atom) -> std::vector<Atom *> 
	{
		struct AtomDist
		{
			Atom *partner = nullptr;
			float distsq = FLT_MAX;
		};
		
		std::vector<AtomDist> dists;
        float min_dsq = bond_len * bond_len;

		for (Atom *other : atoms)
		{
			glm::vec3 vec = (atom->otherPosition("target") - 
			                 other->otherPosition("target"));

			float dsq = glm::dot(vec, vec);
			
            if (dsq <= min_dsq)
            {
                AtomDist datum;
                datum.partner = other;
                datum.distsq = dsq;
                dists.push_back(datum);
            }
		}
        
        std::sort(dists.begin(), dists.end(), [](const AtomDist &a, const AtomDist &b)
		{
            return a.distsq < b.distsq;
		}); // IDK if this matches the house style?
		
		std::vector<Atom *> ret; // HELEN: Change to std::vector
		for (const auto &d : dists)
		{
            // Future conditions for selecting 4 fiducials.
            ret.push_back(d.partner);
		}

		return ret;
	};
};

void NonCovalents::findInterfaces(const std::function<int(Atom *const &)> 
                                  &atom_idx)
{
	for (Segment &first : _segments)
	{
		for (Segment &second : _segments)
		{
			if (first == second) continue;

			Interface face = findInterface(first, second);
			if (face.lefts.atoms.size() == 0 ||
			    face.rights.atoms.size() == 0)
			{
				continue;
			}
			
			Interface::Side &l = face.lefts;
			Interface::Side &r = face.rights;
			l.seq_idxs = l.atoms.convert_to_vector<int>(atom_idx);
			l.reindex();
			r.seq_idxs = r.atoms.convert_to_vector<int>(atom_idx);
			r.reindex();

			if (first < second)
			{
				_connectionPairs.insert(_connectionPairs.end(),
				                        face.atom_pairs.begin(),
				                        face.atom_pairs.end());
			}
			
			_faces.push_back(face);
		}
	}
}

glm::vec3
NonCovalents::WeightedSum::position_for_weights(const GetPos &getPos,
                                                const OpVec<float> &weights)
{
	glm::vec3 ave{};

	for (Atom *fid : fiducials)
	{
		glm::vec3 pos = getPos(fid);
		ave += pos;
	}
	
	ave /= (float)fiducials.size();

	int n = 0;
	glm::vec3 sum{};

	for (Atom *fid : fiducials)
	{
		glm::vec3 pos = getPos(fid);
		pos -= ave;
		pos *= weights[n];
		n++;
		sum += pos;
	}
	
	return sum + ave;
}

OpVec<float>
NonCovalents::WeightedSum::weights_for_positions(const GetPos &getPos) // Helen: optimize
{
	glm::vec3 ave{};
	
	if (fiducials.size() < 4)
	{
		return {};
	}

	std::vector<glm::vec3> positions;
	positions.reserve(fiducials.size());
	
	for (Atom *fid : fiducials)
	{
		glm::vec3 pos = getPos(fid);
		positions.push_back(pos);
		ave += pos;
	}
	
	ave /= (float)fiducials.size();

	Eigen::MatrixXf to_centric(4, fiducials.size());
	for (size_t i = 0; i < positions.size(); i++)
	{
		to_centric(0, i) = positions[i].x - ave.x;
		to_centric(1, i) = positions[i].y - ave.y;
		to_centric(2, i) = positions[i].z - ave.z;
		to_centric(3, i) = 1.0f;
	}

	glm::vec3 q = getPos(atom);
	Eigen::Vector4f vec(q.x - ave.x, q.y - ave.y, q.z - ave.z, 0.0f);

	Eigen::MatrixXf weights = to_centric.fullPivLu().solve(vec);

	/*
	Eigen::MatrixXf weights;
	if (fiducials.size() == 4)
	{
		weights = to_centric.partialPivLu().solve(vec);
	}
	else
	{
		weights = to_centric.colPivHouseholderQr().solve(vec);
	}
	*/

	/*
    {
        std::cout << "SOLVE: " << std::endl;
        std::cout << to_centric << std::endl;
        std::cout << "EQUALS: " << std::endl;
        std::cout << vec << std::endl;
        std::cout << "WEIGHTS: " << std::endl;
        std::cout << weights << std::endl;
        std::cout << std::endl;
    }
    */

    OpVec<float> ws(weights.rows());
	for (int i = 0; i < weights.rows(); i++)
	{
		ws[i] = weights(i, 0);
	}

	return ws;
}

NonCovalents::WeightedSum::WeightedSum(Atom *a, 
                                       const std::vector<Atom *> &as)
{
	atom = a;
	fiducials = as;
	
	auto start_pos = [](Atom *a)
	{
//		std::cout << "start: " << a->desc() << " " << a->otherPosition("target") << std::endl;
		return a->otherPosition("target");
	};
	auto end_pos = [](Atom *a)
	{
//		std::cout << "end: " << a->desc() << " " << a->otherPosition("end") << std::endl;
		return a->otherPosition("end");
	};

	OpVec<float> start_weights = weights_for_positions(start_pos);
	OpVec<float> end_weights = weights_for_positions(end_pos);
	if (start_weights.size() < 4 || end_weights.size() < 4)
	{
		ave_weight = -1;
		return;
	}

	OpVec<float> diffs = end_weights - start_weights;
	
	ave_weight = 0;
	weight_variance = 0;
	
	size_t count = 0;
	for (float &f : start_weights)
	{
		start_weight_cache.push_back(f);
		ave_weight += fabs(f);
		weight_variance += diffs[count] * diffs[count];
		count++;
	}

    ave_weight /= (float)start_weights.size();
    weight_variance /= (float)start_weights.size();
    //std::cout << "AVERAGE = " << ave_weight << std::endl;
    //std::cout << "VARIANCE = " << weight_variance << std::endl;

    weights_for_frac = [start_weights, diffs](float frac)
	{
		return start_weights + diffs * frac;
    };
}

template <typename GetPos>
glm::vec3 average_atoms_position(const std::vector<Atom *> &atoms,
                                 GetPos getPos)
{
	glm::vec3 ave{};

	for (Atom *a : atoms)
	{
		glm::vec3 pos = getPos(a);
		ave += pos;
	}

	if (!atoms.empty())
	{
		ave /= (float)atoms.size();
	}
	
	return ave;
}

template <typename GetPos>
bool is_coplanar(const std::vector<Atom *> &neighbours, GetPos getPos,
                 float threshold = 0.1f, bool log = false)
{
	if (neighbours.size() < 4)
	{
		return true;
	}

	Eigen::MatrixXf posmat(3, 3);
	glm::vec3 avg_pos = average_atoms_position(neighbours, getPos);

	posmat.setZero();
	for (Atom *a : neighbours)
	{
    glm::vec3 derivpos = getPos(a) - avg_pos;

    for (int i = 0; i < 3; i++)
	{
      for (int j = 0; j < 3; j++)
	  {
        posmat(i, j) += derivpos[i] * derivpos[j];
      }
    }
  }

  // Eigen::MatrixXf pos_matrix(3, 4);

  // int col = 0;
  // for (Atom *fid : neighbours)
  // {
  // 	glm::vec3 pos = fid->derivedPosition();
  // 	pos_matrix(0, col) = pos.x - avg_fid.x;
  // 	pos_matrix(1, col) = pos.y - avg_fid.y;
  // 	pos_matrix(2, col) = pos.z - avg_fid.z;
  // 	col++;
  // }

  // // Calculate the 3x3 covariance matrix: (W * W^T)
  // Eigen::MatrixXf covsvd = pos_matrix * pos_matrix.transpose();
  // std::cout << "\n" << "COVSVD: " << covsvd;

	Eigen::JacobiSVD<Eigen::MatrixXf> svd(posmat, Eigen::ComputeFullU | Eigen::ComputeFullV);
	bool svdplane = (svd.singularValues()(2) < threshold);

	if (log)
	{
		std::cout << "\n" << "POSMAT: " << posmat;

		std::cout << "\n"
				  << "SVD Singular values: "
				  << svd.singularValues().transpose();
				  
		std::cout << (svdplane ? ", are coplanar." : ", are NOT coplanar.")
				  << std::endl;
    }

	return svdplane;
}

void weighted_sums_for_side(NonCovalents::Interface &face, 
                            NonCovalents::Interface::Side &lefts, 
                            NonCovalents::Interface::Side &rights)
{
	auto target_pos = [](Atom *a)
	{
		return a->otherPosition("target");
	};
	auto derived_pos = [](Atom *a)
	{
		return a->derivedPosition();
	};
    // Could use "std::function<glm::vec3(Atom *)>" instead of auto. Should I?

    struct AtomScore
	{
		NonCovalents::WeightedSum candidate;
		float stability_penalty;
		float sparsity_penalty;
		float total_penalty;
	};

	std::vector<AtomScore> scores;
    const float MAX_VARIANCE = 1.0f;
	const float fiducial_coplanar_threshold = 0.01f;
	const float right_coplanar_threshold = 0.1f;
	
	for (Atom *right : rights.atoms)
	{
		auto l = closest_atoms(lefts.atoms, 10.0f);
		std::vector<Atom *> neighbours = l(right);
		
		if (neighbours.size() < 4)
		{
			continue;
		}
		
        NonCovalents::WeightedSum pool_candidate =
        NonCovalents::WeightedSum(right, neighbours);

		std::cout << "\t" << "Candidate atomos: " << pool_candidate.atom->desc() << " <-> ";
		for (Atom *f : pool_candidate.fiducials)
		{
			std::cout << f->desc() << ", ";
		}

		std::vector<Atom *> best_fiducials;
		float best_subset_score = FLT_MAX;
		bool found = false;
		
		for (size_t i = 0; i < neighbours.size() - 3; i++) // Helen: Kinetic trap, figuratively
		{
			for (size_t j = i + 1; j < neighbours.size() - 2; j++)
			{
				for (size_t k = j + 1; k < neighbours.size() - 1; k++)
				{
					for (size_t m = k + 1; m < neighbours.size(); m++) // Helen: Lambda func to absorb the 4 for loops for readability.
					{
						std::vector<Atom *> subset = {neighbours[i], neighbours[j], neighbours[k], neighbours[m]};
						// Helen: Weight_variance check for stability
						
						if (!is_coplanar(subset, derived_pos, fiducial_coplanar_threshold))
						// Minimise is_coplanar usage; make it last check.
						{
							auto get_w = [&](Atom *a)
							{
								for (size_t l = 0; l < neighbours.size(); l++)
								{
									if (neighbours[l] == a) return pool_candidate.start_weight_cache[l];
								}
								return 0.0f;
							};
							
							float sum = 0;
							for (Atom *a : subset) sum += fabs(get_w(a));
							float mean = sum / 4.0f;
							float var = 0;
							for (Atom *a : subset)
							{
								float d = fabs(get_w(a)) - mean;
								//float d = fabs(get_w(a)) - pool_candidate.ave_weight;
								var += d * d;
							}

							if (var < best_subset_score)
							{
								best_subset_score = var;
								best_fiducials = subset;
								found = true;
							}

							if (found && best_subset_score < 0.001f) break; 
						}
					}
					if (found && best_subset_score < 0.001f) break;
				}
				if (found && best_subset_score < 0.001f) break;
			}
			if (found && best_subset_score < 0.001f) break;
		}
		
		if (!found)
		{
			std::cout << "REJECTED (unoptimized 4-set)" << std::endl;
			continue;
		}
		
		NonCovalents::WeightedSum candidate = NonCovalents::WeightedSum(right, best_fiducials);

		std::cout << "ave_weight = " << candidate.ave_weight;
		std::cout << ", weight_variance = " << candidate.weight_variance;

		if (candidate.ave_weight < 5 && candidate.ave_weight >= 0 && 
		    candidate.weight_variance < MAX_VARIANCE)
		{
			if (candidate.ave_weight != candidate.ave_weight ||
			    !isfinite(candidate.ave_weight))
			{
				std::cout << ", REJECTED (invalid)" << std::endl;
				continue;
			}
			
			std::cout << ", preliminarily ACCEPTED" << std::endl;

			AtomScore score = {candidate, 0.0f, 0.0f};
			scores.push_back(score);
		}
		else
		{
			std::cout << ", REJECTED (failed weights criteria)" << std::endl;
		}
	}

	std::cout << "\n" << scores.size() << " candidates before penalty." << std::endl;

	if (scores.empty()) return;

	std::vector<AtomScore> active_scores = scores;

	std::cout << "Optimizing selection from " << active_scores.size() << " right atoms." << std::endl;

    bool use_min_distance = true;
	
	// HELEN: Function encapsulates dynamic penalty calculation and pruning logic
	auto recalculate_and_prune = [&]() -> bool
	{
		for (size_t i = 0; i < active_scores.size(); i++)
		{
			float min_dist_sq = FLT_MAX;
			float sum_dist_sq = 0;
			int count = 0;
			glm::vec3 posA = target_pos(active_scores[i].candidate.atom);
			
			for (size_t j = 0; j < active_scores.size(); j++)
			{
				if (i == j) continue;
				glm::vec3 posB = target_pos(active_scores[j].candidate.atom);
				float dist_sq = glm::dot(posA - posB, posA - posB);
				if (dist_sq < min_dist_sq) min_dist_sq = dist_sq;
				sum_dist_sq += dist_sq;
				count++;
			}
			
			float mean_dist_sq = (count > 0) ? (sum_dist_sq / count) : 1.0f;
			
			if (use_min_distance)
			{
				active_scores[i].sparsity_penalty = 1.0f / min_dist_sq;
			}
			else
			{
				active_scores[i].sparsity_penalty = 1.0f / mean_dist_sq;
			}
			
			active_scores[i].total_penalty = active_scores[i].sparsity_penalty + active_scores[i].stability_penalty;
		}

		std::sort(active_scores.begin(), active_scores.end(), [](const AtomScore &a, const AtomScore &b)
		{
			return a.total_penalty > b.total_penalty;
		});

		for (auto it = active_scores.begin(); it != active_scores.end(); ++it)
		{
			if (it->total_penalty < 0.6f)
			{
				return false;
			}
			
			std::vector<Atom *> test_pool;
			for (auto test_it = active_scores.begin(); test_it != active_scores.end(); ++test_it)
			{
				if (test_it != it)
				{
					test_pool.push_back(test_it->candidate.atom);
				}
			}
			
			bool coplanar_choice = is_coplanar(test_pool, target_pos, right_coplanar_threshold);
			
			if (!coplanar_choice)
			{
				std::cout << "\tRemoved worst atom: " << it->candidate.atom->desc() 
						  << " (penalty: " << it->total_penalty << ")" << std::endl;
				
				active_scores.erase(it);
				return true;
			}
			else
			{
				std::cout << "\tKept atom (needed for planarity): " << it->candidate.atom->desc() << std::endl;
			}
		}
		
		return false;
	};

	while (active_scores.size() > 4)
	{
		if (!recalculate_and_prune()) break;
	}

	for (auto &s : active_scores)
	{
		s.candidate.ave_weight = 1;
		face.sums.push_back(s.candidate);
	}

	std::cout << "Final selection size: " << face.sums.size() << std::endl;
}

void NonCovalents::prepareBarycentricWeights()
{
	auto col_idx_for_seq_idx = [this](int idx)
	{
		return _seqToId.count(idx) ? _seqToId.at(idx).col : -1;
	};

	int size = _positions.cols();
	for (Interface &face : _faces)
	{
		weighted_sums_for_side(face, face.lefts, face.rights);

		if (face.left < face.right)
		{
			for (WeightedSum &sum : face.sums)
			{
				for (Atom *fiducial : sum.fiducials)
				{
					_fiducialConnectionPairs.push_back({sum.atom, fiducial});
				}
			}
		}

		auto seq_idx_for_atom = [face](Atom *atom)
		{
			if (face.lefts.locs.count(atom))
			{
				return face.lefts.seq_idxs[face.lefts.locs.at(atom)];
			}
			if (face.rights.locs.count(atom))
			{
				return face.rights.seq_idxs[face.rights.locs.at(atom)];
			}
			return -1;
		};
		
		auto col_idx_for_atom = [seq_idx_for_atom, col_idx_for_seq_idx](Atom *a)
		{
			return col_idx_for_seq_idx(seq_idx_for_atom(a));
		};
		
		for (WeightedSum &sum : face.sums)
		{
			sum.weights_to_matrix_column = 
			[size, col_idx_for_atom, &sum](float frac)
			{
				Eigen::VectorXf vec(size);
				vec.setZero();
				OpVec<float> weights = sum.weights_for_frac(frac);

				// insert all participating atoms' positions
				int n = 0;
				float tot = 0;
				float ave_contrib = 1 / (float)sum.fiducials.size();
				for (Atom *fid : sum.fiducials)
				{
					int col = col_idx_for_atom(fid);
					const float &weight = weights[n];
					vec[col] = weight + ave_contrib;
					n++;
				}
				
				// subtract calculated atom's position to aim for zero
				int col = col_idx_for_atom(sum.atom);
				vec[col] = -1;
				vec *= sum.ave_weight;

				return vec;
			};
		}
	}

	
	OpSet<Atom *> fixed_atoms = total_invariant_atoms(_faces, 
	                                                  _invariant);
	std::vector<int> columns_for_fixed; 
	columns_for_fixed.reserve(fixed_atoms.size());

	for (Atom *atom : fixed_atoms)
	{
		int b_idx = _atom2Seq[atom];
		int col_idx = _seqToId[b_idx].col;
		columns_for_fixed.push_back(col_idx);
	}
	
	_weightsToMatrixPositions = [this, columns_for_fixed, size]
	(const float &frac, Eigen::MatrixXf &dest)
	{
		int n = 0;
		for (Interface &face : _faces)
		{
			for (WeightedSum &sum : face.sums)
			{
				Eigen::VectorXf vec = sum.weights_to_matrix_column(frac);
				dest(Eigen::all, n) = vec;
				n++;
			}
		}
		
		for (int fixed : columns_for_fixed)
		{
			if (n >= _barycentrics.cols())
			{
				break;
			}
			Eigen::VectorXf vec(size);
			vec.setZero();
			vec[fixed] = 1;
			dest(Eigen::all, n) = vec;
			n++;
		}
	};
}

void NonCovalents::prepare(BondSequence *const &seq)
{
	_atomNumbers.clear();
	_segments.clear();
	_segment2Idx.clear();
	_faces.clear();
	_connectionPairs.clear();
	_fiducialConnectionPairs.clear();
	_seqToId.clear();
	_atom2Seq.clear();
	_matIds.clear();
	_invariant = Segment{-1};
	_snapColumnFrom = 0;

	auto atom_index = [seq](Atom *const &atom) -> int
	{
		return atom_index_for_atom(seq, atom);
	};
	
	// each instance is provided a list of AtomBlock indices.
	// currently does not account for chain breaks - needs retrofitting.
	assignSegmentsToAtoms(seq);
	
	// all atoms are inspected for involvement at an instance-instance
	// interface and a list made of participating atoms for each interface.
	findInterfaces(atom_index);

	// enough information now to prepare the matrix sizes.
	preparePositionMatrix();
	
	// decide which participating atoms go into which columns for matrix
	prepareCoordinateColumns(atom_index);

	// prepare the barycentric coordinates for each atom!
	prepareBarycentricWeights();
	
	// prepare the barycentric matrix template.
	prepareBarycentricTargetMatrices();

	// prepare the target weights from invariant instance
	prepareTargets(atom_index);
}

template <class T>
std::map<T, int> to_indices(std::vector<T> &ts)
{
	std::map<T, int> ret;
	int idx = 0;
	for (T t : ts)
	{
		ret[t] = idx;
		idx++;
	}

	return ret;
}

void NonCovalents::Interface::Side::reindex()
{
	std::vector<Atom *> vec = atoms.toVector();
	locs = to_indices(vec);
}

std::vector<NonCovalents::MatId> 
NonCovalents::matrix_coordinates(const OpSet<Atom *> &all,
                                 const std::function<int(Atom *const &)> 
                                 &atom_idx)
{
	std::vector<Atom *> vec = all.toVector();
	std::vector<int> seqs = all.convert_to_vector<int>(atom_idx);
	std::map<Atom *, int> locate = to_indices(vec);

	auto get_row = [this](Segment &seg)
	{	
		return _segment2Idx.count(seg) ? 4 * _segment2Idx.at(seg) : -1;
	};

	auto get_col = [locate](Atom *const &atom)
	{
		return locate.count(atom) ? locate.at(atom) : -1;
	};

	std::vector<MatId> ids; ids.reserve(vec.size());
	
	for (Atom *const &atom : vec)
	{
		Segment chosen{-1};
		for (Segment segment : _segments)
		{
			if (atomBelongsToSegment(atom, segment))
			{
				chosen = segment;
			}
		}
		if (chosen.num < 0)
		{
			continue;
		}

		int row = get_row(chosen);
		int col = get_col(atom);
		int seq = seqs[col];
		bool inv = (chosen == _invariant);
		
		std::cout << chosen.grp->size() << "," << chosen.num << " -> ";
		std::cout << row << ", " << col << ", " << seq << ", " <<
		(inv ? "fixed" : "variable") << std::endl;

		ids.push_back({row, col, seq, inv});
	}
	
	return ids;
}

OpSet<Atom *> total_atoms(const std::vector<NonCovalents::Interface> &faces)
{
	int total = 0;
	OpSet<Atom *> all;
//	std::cout << "Total atoms: " << std::endl;
	for (const NonCovalents::Interface &face : faces)
	{
//		if (face.left != invariant)
		{
			all += face.lefts.atoms;
		}
//		if (face.right != invariant)
		{
			all += face.rights.atoms;
		}
	}
	return all;
}

void NonCovalents::prepareBarycentricTargetMatrices()
{
	int l = _positions.rows();
	int n = _positions.cols();
	int m = 0;

	OpSet<Atom *> invariants = total_invariant_atoms(_faces, _invariant);
	
	for (Interface &face : _faces)
	{
		m += face.sums.size();
	}
	
	_snapColumnFrom = m;
	
//	m += 4;
	m += invariants.size();

	_barycentrics = MatrixXf(n, m);
	_barycentrics.setZero();
	_targets = MatrixXf(3, m);
	_targets.setZero();

	std::cout << "Dimensions: " << l << ", " << n << ", " << m << std::endl;
}

void NonCovalents::preparePositionMatrix()
{
	int l = _segments.size() * 4;
	int n = total_atoms(_faces).size();

	_positions = MatrixXf(l, n);
	_positions.setZero();
}
	
void NonCovalents::
	prepareCoordinateColumns(const std::function<int(Atom *const &)> &atom_idx)
{
	OpSet<Atom *> all = total_atoms(_faces);
	std::vector<MatId> ids = matrix_coordinates(all, atom_idx);
	std::cout << "position ids: " << ids.size() << std::endl;
	
	for (const MatId &id : ids)
	{
		_seqToId[id.idx] = id;
	}

	auto get_row = [this](Segment &seg)
	{	
		return _segment2Idx.count(seg) ? 4 * _segment2Idx.at(seg) : -1;
	};

	int fixed_row = get_row(_invariant);
	
	_blocksToMatrixPositions = [this, ids, fixed_row]
	(BondSequence *seq, Eigen::MatrixXf &dest, bool trans_only)
	{
		for (const MatId &id : ids)
		{
			if (!trans_only)
			{
				const glm::vec3 &p = seq->blocks()[id.idx].my_position();
				Eigen::Vector3f vec(p.x, p.y, p.z);
				dest(Eigen::seqN(id.row, 3), id.col) = vec;
			}
			dest(id.row + 3, id.col) = 1;
		}
	};
	
	_snapToTargetColumns = [ids, this]
	(BondSequence *seq, Eigen::MatrixXf &dest)
	{
		int n{};
		for (const MatId &id : ids)
		{
			if (!id.fixed)
			{
				continue;
			}

			int t = _snapColumnFrom + n;
			if (t >= _targets.cols())
			{
				continue;
			}
			const glm::vec3 &p = seq->blocks()[id.idx].my_position();
			Eigen::Vector3f vec(p.x, p.y, p.z);
			dest(Eigen::seqN(id.row, 3), t) = vec;
			n++;
		}
	};

}

void NonCovalents::prepareTargets(const std::function<int(Atom *const &)> 
                                  &atom_idx)
{	
}

glm::mat4x4 eigenMat4x3ToGlm(Eigen::MatrixXf &mat)
{
	glm::mat4x4 transform{};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			transform[i][j] = mat(i, j);
		}
	}

	transform[3][3] = 1;

	return transform;
}

std::function<BondSequence *(BondSequence *)> 
NonCovalents::align(const float &frac)
{
	auto job = [this, frac](BondSequence *seq) -> BondSequence *
	{
		// get local copies of matrix templates
		Eigen::MatrixXf positions = _positions;
		Eigen::MatrixXf translations = _positions;
		Eigen::MatrixXf barycentrics = _barycentrics;
		Eigen::MatrixXf b = _targets;
		
		_blocksToMatrixPositions(seq, positions, false);
		_blocksToMatrixPositions(seq, translations, true);
		_weightsToMatrixPositions(frac, barycentrics);
		_snapToTargetColumns(seq, b);
		
//		write_to_file(positions, "positions.2d");
//		write_to_file(barycentrics, "barycentrics.2d");
//		write_to_file(b, "targets.2d");
		
		Eigen::MatrixXf A = positions * barycentrics;
//		write_to_file(A, "multiplied.2d");

		/*
		std::cout << "positions = " << positions.rows() << " x " << positions.cols() << " and "
		<< "barycentrics = " << barycentrics.rows() << " x " << barycentrics.cols() <<  std::endl;
		std::cout << std::endl;

		std::cout << "positions: " << std::endl << positions << std::endl;
		std::cout << "barycentrics: " << std::endl << barycentrics << std::endl;

		std::cout << "A: " << std::endl << A << std::endl;
		std::cout << "targets: " << std::endl << b << std::endl;
		std::cout << std::endl;
		*/

		Eigen::BDCSVD<Eigen::MatrixXf> svd(A.transpose(), 
		                                   Eigen::ComputeThinU | Eigen::ComputeThinV);
		MatrixXf sol = svd.solve(b.transpose());
//		write_to_file(sol, "solution.2d");

		/*
		std::cout << "Sol, pre-clean: " << std::endl;
		std::cout << sol << std::endl;
		std::cout << std::endl;
		*/

		// now we need to re-calculate the translations needed.
		Eigen::MatrixXf check = sol.transpose() * A;
//		write_to_file(check, "check.2d");

		/*
		std::cout << "pre-clean check:\n" << check << std::endl;
		std::cout << std::endl;
		*/

		// performing SVD on each mini-3x3 matrix in order to
		// extract a proper rotation matrix.
		Eigen::Vector3f empty;
		empty.setZero();

		int n = 0;
		for (int i = 0; i < _segments.size(); i++)
		{
			if (_segment2Idx.count(_segments[i]) == 0)
			{
				continue; // invariant
			}

			int j = n * 4; 
			n++;

			Eigen::Matrix3f cutout = sol(Eigen::seqN(j, 3), {0, 1, 2});

			Eigen::JacobiSVD<MatrixXf> svd(cutout, Eigen::ComputeFullU | 
			                               Eigen::ComputeFullV);
			Eigen::MatrixXf u = svd.matrixU();
			Eigen::MatrixXf v = svd.matrixV();
			Eigen::MatrixXf fixed = u * v.transpose();
			if (fixed.determinant() < 0)
			{
				v({0, 1, 2}, 2) *= -1.f;
				fixed = u * v.transpose();
			}

			sol(Eigen::seqN(j, 3), {0, 1, 2}) = fixed;
		}
		
		/*
		std::cout << "sol (rot matrix correction):\n" << sol << std::endl;
		*/

		// now we need to re-calculate the translations needed.
		Eigen::MatrixXf result = sol.transpose() * A;
//		write_to_file(result, "result.2d");
		/*
		std::cout << "rot check:\n" << result << std::endl;
		std::cout << std::endl;
		*/

		Eigen::MatrixXf diff = result - b;
		Eigen::MatrixXf offsets = translations * barycentrics;
		Eigen::MatrixXf trans_sol;
		trans_sol = 
		offsets.transpose().colPivHouseholderQr().solve(diff.transpose());
		sol -= trans_sol;

		// now we need to re-calculate the translations needed.
		check = sol.transpose() * A;
		/*
		std::cout << "trans check:\n" << check << std::endl;
		std::cout << std::endl;
		
		*/

		std::map<Segment, glm::mat4x4> rots;

		n = 0;
		for (int i = 0; i < _segments.size(); i++)
		{
			if (_segment2Idx.count(_segments[i]) == 0)
			{
				continue; // invariant
			}

			int j = n * 4;  n++;

			Eigen::MatrixXf cutout = sol(Eigen::seqN(j, 4), {0, 1, 2});

			Segment seg = _segments[i];
			rots[seg] = eigenMat4x3ToGlm(cutout);
		}

		for (Segment &seg : _segments)
		{
			std::vector<int> &idxs = _atomNumbers[seg];
			if (rots.count(seg) == 0)
			{
				continue;
			}
			const glm::mat4x4 &transform = rots.at(seg);

			for (const int &idx : idxs)
			{
				mat4x4 &basis = seq->blocks()[idx].basis;
				vec4 tmp = basis[3]; tmp[3] = 1.;
				basis[3] = transform * tmp;
			}
		}
		return seq;
	};
	
	return job;
}

std::function<BondSequence *(BondSequence *)> 
NonCovalents::align_task(const float &frac)
{
	auto alignment = align(frac);

	return alignment;
}

std::set<ScoreBucket> NonCovalents::buckets()
{
	std::set<ScoreBucket> buckets;
	for (Segment &segment : _segments)
	{
		int min, max;
		AtomGroup *group = segment.grp;
		std::string chain = group->chosenAnchor()->chain();
		group->getLimitingResidues(&min, &max);
		buckets.insert(ScoreBucket{chain, min, max});
	}

	return buckets;
}
