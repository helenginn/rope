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
			else
			{
				_segment2Idx[*segment] = m;
				m++;
			}

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
		const float max = 3.5f;
		bool bad = false;
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

			interface.lefts.atoms.insert(atom);
			interface.rights.atoms.insert(other);
		}
	}
	
	return interface;
}

auto closest_atoms(OpSet<Atom *> &atoms, int total)
{
	return [&atoms, total](Atom *const &atom) -> OpSet<Atom *> 
	{
		struct AtomDist
		{
			Atom *partner = nullptr;
			float distsq = FLT_MAX;
		};
		
		AtomDist *dist = new AtomDist[total];

		for (Atom *other : atoms)
		{
			glm::vec3 vec = (atom->otherPosition("target") - 
			                 other->otherPosition("target"));

			float dsq = glm::dot(vec, vec);
			
			for (int i = 0; i < total; i++)
			{
				if (dsq < dist[i].distsq)
				{
					for (int j = total - 1; j >= i + 1; j--)
					{
						dist[j] = dist[j - 1];
					}

					dist[i].distsq = dsq;
					dist[i].partner = other;
					break;
				}
			}
		}
		
		OpSet<Atom *> ret;
		for (int i = 0; i < total; i++)
		{
			if (dist[i].partner)
			{
				ret += dist[i].partner;
			}
		}

		delete [] dist;
		return ret;
	};
};

void NonCovalents::findInterfaces(const std::function<int(Atom *const &)> 
                                  &atom_idx)
{
	for (Segment &first : _segments)
	{
		if (first == _invariant) { continue; }

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
NonCovalents::WeightedSum::weights_for_positions(const GetPos &getPos)
{
	glm::vec3 ave{};
	
	if (fiducials.size() < 4)
	{
		return {};
	}

	Eigen::MatrixXf to_centric(fiducials.size(), 4);
	int n = 0;
	for (Atom *fid : fiducials)
	{
		glm::vec3 pos = getPos(fid);
		to_centric(n, Eigen::seq(0, 2)) = Eigen::Vector3f(pos.x, pos.y, pos.z);
		to_centric(n, 3) = 1;
		ave += pos;
		n++;
	}
	
	ave /= (float)fiducials.size();
	for (int i = 0; i < n; i++)
	{
		to_centric(i, Eigen::seq(0, 2)) -= Eigen::Vector3f(ave.x, ave.y, ave.z);
	}

	to_centric.transposeInPlace();
	glm::vec3 q = getPos(atom);
	Eigen::Vector4f vec(4);
	vec = {q.x - ave.x, q.y - ave.y, q.z - ave.z, 0};

	Eigen::MatrixXf weights = to_centric.colPivHouseholderQr().solve(vec);
	
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

	std::vector<float> ws = {weights(0), weights(1), weights(2), weights(3)};
	return OpVec<float>(ws);
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
	for (float &f : start_weights)
	{
		ave_weight += fabs(f);
	}
	ave_weight /= (float)start_weights.size();

	weights_for_frac = [start_weights, diffs](float frac)
	{
		return start_weights + diffs * frac;
	};
}

void weighted_sums_for_side(NonCovalents::Interface &face, 
                            NonCovalents::Interface::Side &lefts, 
                            NonCovalents::Interface::Side &rights)
{
	for (Atom *right : rights.atoms)
	{
		auto l = closest_atoms(lefts.atoms, 4);
		std::vector<Atom *> neighbours = (l(right)).toVector();

		NonCovalents::WeightedSum candidate = 
		NonCovalents::WeightedSum(right, neighbours);

		if (candidate.ave_weight < 5 && candidate.ave_weight >= 0)
		{
			candidate.ave_weight = 1 / (candidate.ave_weight *
			                            candidate.ave_weight);
			if (candidate.ave_weight != candidate.ave_weight ||
			    !isfinite(candidate.ave_weight))
			{
				continue;
			}
			
			std::cout << "\t" << candidate.atom->desc() << " <-> ";
			
			for (Atom *f : candidate.fiducials)
			{
				std::cout << f->desc() << ", ";
			}
			std::cout << std::endl;

			candidate.ave_weight = 1;
			face.sums.push_back(candidate);
		}
	}
}

void NonCovalents::prepareBarycentricWeights()
{
	auto col_idx_for_seq_idx = [this](int idx)
	{
		return _seqToId.count(idx) ? _seqToId.at(idx).col : -1;
	};

	for (Interface &face : _faces)
	{
		weighted_sums_for_side(face, face.lefts, face.rights);

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
			int size = _positions.cols();
			bool invariant = (face.right == _invariant);
			sum.weights_to_matrix_column = 
			[size, col_idx_for_atom, &sum, invariant](float frac)
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
				
				if (!invariant)
				{
					vec[col] = -1;
				}

				vec *= sum.ave_weight;

				return vec;
			};
		}
	}
	
	_weightsToMatrixPositions = [this](const float &frac, 
	                                   Eigen::MatrixXf &dest)
	{
		int n = 0;
		for (Interface &face : _faces)
		{
			if (face.sums.size() < 3)
			{
//				continue;
			}

			for (WeightedSum &sum : face.sums)
			{
				Eigen::VectorXf vec = sum.weights_to_matrix_column(frac);
				dest(Eigen::all, n) = vec;
				n++;
			}
		}
	};
}

void NonCovalents::prepare(BondSequence *const &seq)
{
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

	// prepare the barycentric coordinates for each atom!
	prepareBarycentricWeights();
	
	// prepare the barycentric matrix template.
	prepareBarycentricTargetMatrices();
	
	// decide which participating atoms go into which columns for matrix
	prepareCoordinateColumns(atom_index);

	// prepare the target weights from invariant instance
	prepareTargets();
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
NonCovalents::target_coordinates()
{
	std::vector<MatId> ids; ids.reserve(_barycentrics.rows());
	int n = 0;
	
	for (Interface &face : _faces)
	{
		for (WeightedSum &sum : face.sums)
		{
			MatId id = {-1, -1, -1};
			if (face.right == _invariant)
			{
				Atom *a = sum.atom;
				int seq = face.rights.seq_idxs[face.rights.locs[a]];
				id.col = n;
				id.idx = seq;
				id.weight = sum.ave_weight;

			}
			ids.push_back(id);
			n++;
		}
	}
	
	return ids;
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
	
//	std::cout << "Invariant: " << _invariant->id() << std::endl;
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
		if (chosen.num < 0) { continue; }

		int row = get_row(chosen);
		int col = get_col(atom);
		int seq = seqs[col];
		
		std::cout << chosen.grp->size() << "," << chosen.num << " -> ";
		std::cout << row << ", " << col << ", " << seq << std::endl;

		ids.push_back({row, col, seq});
	}
	
	return ids;
}

OpSet<Atom *> total_atoms(const std::vector<NonCovalents::Interface> &faces,
                          Segment &invariant)
{
	int total = 0;
	OpSet<Atom *> all;
//	std::cout << "Total atoms: " << std::endl;
	for (const NonCovalents::Interface &face : faces)
	{
		std::cout << " + " << face.lefts.atoms.size() << std::endl;
		all += face.lefts.atoms;
		if (face.right != invariant)
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
	
	for (Interface &face : _faces)
	{
//		if (face.sums.size() >= 3)
		{
			m += face.sums.size();
		}
	}

	_barycentrics = MatrixXf(n, m);
	_barycentrics.setZero();
	_targets = MatrixXf(3, m);
	_targets.setZero();

	std::cout << "Dimensions: " << l << ", " << n << ", " << m << std::endl;
}

void NonCovalents::preparePositionMatrix()
{
	int l = (_segments.size() - 1) * 4;
	int n = total_atoms(_faces, _invariant).size();

	_positions = MatrixXf(l, n);
	_positions.setZero();
}
	
void NonCovalents::
	prepareCoordinateColumns(const std::function<int(Atom *const &)> &atom_idx)
{
	OpSet<Atom *> all = total_atoms(_faces, _invariant);
	std::vector<MatId> ids = matrix_coordinates(all, atom_idx);
	std::cout << "position ids: " << ids.size() << std::endl;
	
	for (const MatId &id : ids)
	{
		_seqToId[id.idx] = id;
	}
	
	_blocksToMatrixPositions = [this, ids](BondSequence *seq, 
	                                       Eigen::MatrixXf &dest,
	                                       bool trans_only)
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
}

void NonCovalents::prepareTargets()
{	
	std::vector<MatId> tids = target_coordinates();
	_matIds = tids;
	std::cout << "target ids: " << tids.size() << std::endl;

	_blocksToTargetMatrix = [tids](BondSequence *seq, 
	                                    Eigen::MatrixXf &dest)
	{
		for (const MatId &id : tids)
		{
			if (id.col >= 0)
			{
				const glm::vec3 &p = seq->blocks()[id.idx].my_position();
				Eigen::Vector3f vec(p.x, p.y, p.z);
				dest(Eigen::all, id.col) = vec * id.weight;
			}
		}
	};
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
		_blocksToTargetMatrix(seq, b);
		
		Eigen::MatrixXf A = positions * barycentrics;

		/*
		std::cout << "positions = " << positions.rows() << " x " << positions.cols() << " and "
		<< "barycentrics = " << barycentrics.rows() << " x " << barycentrics.cols() <<  std::endl;
		std::cout << std::endl;

		std::cout << "positions: " << std::endl << positions << std::endl;
		std::cout << "barycentrics: " << std::endl << barycentrics << std::endl;
		std::cout << std::endl;

		std::cout << "A: " << std::endl << A << std::endl;
		std::cout << "b: " << std::endl << b << std::endl;
		std::cout << std::endl;
		*/

		Eigen::MatrixXf sol;
		sol = A.transpose().colPivHouseholderQr().solve(b.transpose());

		/*
		std::cout << "Sol, pre-clean: " << std::endl;
		std::cout << sol << std::endl;
		std::cout << std::endl;
		*/

		// now we need to re-calculate the translations needed.
		Eigen::MatrixXf check = sol.transpose() * A;
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
//			sol(j + 3, {0, 1, 2}) = empty;
		}
		
		/*
		std::cout << "sol (rot matrix correction):\n" << sol << std::endl;
		*/

		// now we need to re-calculate the translations needed.
		Eigen::MatrixXf result = sol.transpose() * A;
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
		
		std::cout << "Sol * Positions: " << std::endl;
		std::cout << sol * positions << std::endl;
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
			if (seg == _invariant)
			{
				continue;
			}

			std::vector<int> &idxs = _atomNumbers[seg];
			if (rots.count(seg) == 0)
			{
				continue;
			}
			const glm::mat4x4 &transform = rots.at(seg);
//			std::cout << transform << std::endl;

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
