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

#include "PairwiseDeviations.h"
#include "engine/ElementTypes.h"
#include "engine/Task.h"
#include "BondSequence.h"
#include "BundleBonds.h"
#include "LoopRoundResidues.h"
#include "Separation.h"

PairwiseDeviations::PairwiseDeviations(BondSequence *sequence, 
                                       const float &limit, Separation *sep,
                                       bool for_momentum)
{
	_limit = limit;
	_sep = sep;
	_momentum = for_momentum;
	
	prepare(sequence);
}

PairwiseDeviations::~PairwiseDeviations()
{
	delete [] _reference;

}

bool worth_checking_distance(const glm::vec3 &rest_i, const glm::vec3 &rest_j,
                             const glm::vec3 &move_i, const glm::vec3 &move_j,
                             float threshold)
{
	for (int n = 0; n < 3; n++)
	{
		if (fabs(rest_i[n] - rest_j[n]) > threshold) return false;
		if (fabs(move_i[n] - move_j[n]) > threshold) return false;
	}

	return true;
}

void PairwiseDeviations::prepare(BondSequence *seq)
{
	std::cout << "Preparing pairwise deviations" << std::endl;
	const std::vector<AtomBlock> &blocks = seq->blocks();
	glm::vec3 *scratch = new glm::vec3[blocks.size() * 2];
	_atoms = std::vector<Atom *>(blocks.size());

	std::vector<int> gaps;
	int n = 0;
	auto collect_targets = [this, scratch, &n, &gaps]
	(const AtomBlock &block)
	{
		Atom *const &atom = block.atom;
		if (atom)
		{
			const glm::vec3 &orig = atom->otherPosition("target");
			const glm::vec3 &moving = atom->otherPosition("moving");
			
			if (glm::length(orig) < 1e-6)
			{
				std::cout << "!!!" << atom->desc() << " " << orig << std::endl;
			}
			scratch[n] = orig;
			scratch[n + 1] = moving;
		}
		else
		{
			gaps.push_back(n + 2);
		}
		_atoms[n / 2] = atom;
		n += 2;
	};
	
	do_on_each_block(blocks, collect_targets);
	gaps.push_back(n);

	// pre-calculate pairs to interrogate
	
	auto distance_between_atoms = [](const glm::vec3 &at_rest,
	                                 const glm::vec3 &moving, float t)
	{
		glm::vec3 total = at_rest + moving * t;
		return glm::length(total);
	};
	
	std::vector<int> pairs;

	auto loop_over_atoms = [this](int n0, int n1, auto job)
	{
		for (int i = n0; i < n1 - 2; i += 2)
		{
			int m = i / 2;
			Atom *left = _atoms[m];
			if (!left) continue;

			job(left, i);
		}
	};
	
	auto loop_over_atom_pairs = [this](int n0, int n1, auto job)
	{
		for (int i = n0; i < n1 - 2; i += 2)
		{
			int m = i / 2;
			Atom *left = _atoms[m];
			if (!left) continue;

			for (int j = i + 2; j < n1; j += 2)
			{
				int n = j / 2;
				Atom *right = _atoms[n];
				if (!right) continue;
				
				job(left, right, i, j);

			}
		}
	};
	
	auto reserve_sizes = [this](Atom *const &atom, const int &i)
	{
		int p = i / 2;
		_neighbouring[p].reserve(10000);
	};

	auto shrink_sizes = [this](Atom *const &atom, const int &i)
	{
		int p = i / 2;
		_neighbouring[p].shrink_to_fit();
	};

	auto populate_neighbours = 
	[this, scratch]
	(const Separation::BlockRegion &br)
	{
		return [this, &br]
		(Atom *left, Atom *right, int i, int j)
		{
			int p = i / 2; int q = j / 2;

			auto check_pair = [&p, &q, &br, this] (int b)
			{
				// sensitive distance between q and b
				if (_sep)
				{
					int lr = _sep->separationBetween(br, b, q);
					if (lr < 0) { return false; }
					int lc = _sep->separationBetween(br, b, p);
					if (lc < 0 || lc > lr) { return false; }
					int cr = _sep->separationBetween(br, p, q);
					if (cr < 0) { return false; }
					bool reject = (lc + cr - lr > 0);
					if (reject) { return false; }
				}

				return true;
			};

			auto add_pairs = [this, &check_pair](int p, int q)
			{
				std::vector<int> &qPairs = _perIdx[q];
				_neighbouring[p].reserve(_neighbouring[p].size());

				for (const int &pair : qPairs)
				{
					TargetInfo &pInfo = info(pair);
					int m = pInfo.p; int n = pInfo.q;
					int chosen = (m == q) ? n : m;
					if (check_pair(chosen))
					{
						_neighbouring[p].push_back(pair);
					}
				}
			};

			add_pairs(p, q);
			add_pairs(q, p);
		};
	};
	
	auto add_if_acceptable = 
	                          [this, &scratch, 
	                          &distance_between_atoms]
	(Atom *left, Atom *right, int i, int j)
	{
		int m = i / 2; int n = j / 2;
		const float threshold = _limit;

		const glm::vec3 &at_rest_i = scratch[i];
		const glm::vec3 &moving_i = scratch[i + 1];
		
		bool too_close = false;
		for (size_t k = 0; k < left->bondTorsionCount(); k++)
		{
			if (left->bondTorsion(k)->hasAtom(right))
			{
				too_close = true;
			}
		}

		if (!_momentum && too_close)
		{
			return;
		}

		if (_sep && _sep->separationBetween(m, n) < 0)
		{
			return;
		}

		const glm::vec3 &at_rest_j = scratch[j];
		const glm::vec3 &moving_j = scratch[j + 1];

		if (!worth_checking_distance(at_rest_i, at_rest_j, 
		                             moving_i, moving_j, threshold))
		{
			return;
		}

		glm::vec3 at_rest = at_rest_i - at_rest_j;
		glm::vec3 moving = moving_i - moving_j;

		float start = distance_between_atoms(at_rest, moving, 0);
		float end = distance_between_atoms(at_rest, moving, 1);

		bool is_side = !(left->isMainChain() && right->isMainChain());

		float modulate = is_side ? 0.5 : 1.0;

		bool ok = (start < threshold * modulate || 
		           end < threshold * modulate);

		if (!ok)
		{
			return;
		}

		int size = _infoPairs.size();

		_perResidue[ScoreBucket(left)].push_back(size);
		_perResidue[ScoreBucket(right)].push_back(size);
		_perIdx[m].push_back(size);
		_perIdx[n].push_back(size);
		_residues.insert(ScoreBucket(left));
		_residues.insert(ScoreBucket(right));

		bool close = (start < 2 || end < 2);

		if (_pairs.size() % n == 0)
		{
			_pairs.reserve(_pairs.size() + n);
		}

		_pairs.push_back(_infoPairs.size());
		_infoPairs.push_back({m, n, start, end, close});
		_atoms2Info[{left, right}] = size;
		_atoms2Info[{right, left}] = size;
	};
	
	std::cout << "Segments: " << gaps.size() - 1 << std::endl;
	for (int i = 0; i < gaps.size() - 1; i++)
	{
		int n0 = gaps[i];
		int n1 = gaps[i + 1];
		int p = n0 / 2 + 1;
		Separation::BlockRegion br = {n0/2, n1/2};
		if (_sep)
		{
			br = _sep->group_for_idx(p);
		}
		std::cout << "." << std::flush;

		loop_over_atom_pairs(n0, n1, add_if_acceptable);
		loop_over_atoms(n0, n1, reserve_sizes);
		loop_over_atom_pairs(n0, n1, populate_neighbours(br));
		loop_over_atoms(n0, n1, shrink_sizes);
	}
	std::cout << std::endl;
	
	_reference = scratch;
}


auto simple(PairwiseDeviations *dev, float frac, std::set<ScoreBucket> forResidues)
{
	LoopMechanism loop = loop_mechanism(dev->pairs(), dev->perResiduePairs(), 
	                                    forResidues);

	return [dev, frac, loop]
	(BondSequence *seq) -> Deviation
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		glm::vec3 *scratch = new glm::vec3[blocks.size()];
		
		int n = 0;
		auto collect_targets = [scratch, &n](const AtomBlock &block)
		{
			scratch[n] = block.my_position();
			n++;
		};

		do_on_each_block(blocks, collect_targets);

		float total = 0;
		float count = 0;
		
		target_actual_distances lookup(dev->reference(), scratch);

		auto check_momentum = [&frac, &lookup, &total, &count, dev]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i++)
			{
				TargetInfo &info = dev->info(pairs[i]);
				const int &p = info.p;
				const int &q = info.q;
				if (!dev->filter_in(p) || !dev->filter_in(q))
				{
					continue;
				}

				float targdist = info.target(frac);
				float actualdist = lookup.actual(p, q);
				float weight = 1 / (1 + actualdist);

				float dist = (targdist - actualdist);
				
				total += dist * dist * weight;
				count += weight;
			}
		};

		loop(check_momentum);
		
		total /= (float)count;
		total = sqrt(total);
		if (total != total) total = 0;

		delete [] scratch;

		return {total};
	};
};

Task<BondSequence *, Deviation> *
PairwiseDeviations::momentum_task(float frac, 
                                  const std::set<ScoreBucket> &forResidues)
{
	auto return_deviation = simple(this, frac, forResidues);
	auto *task = new Task<BondSequence *, Deviation>(return_deviation, "momentum");
	return task;
}

Task<BundleBonds *, ActivationEnergy> *
PairwiseDeviations::bundle_clash(const std::set<ScoreBucket> &forResidues)
{
	LoopMechanism loop = loop_mechanism(pairs(), perResiduePairs(), forResidues);

	auto job = [loop, this]
	(BundleBonds *bb) -> ActivationEnergy
	{
		auto lookup = bb->lookup();

		long double total = 0;
		float frac = bb->frac();
		auto check_clashes = [&lookup, &total, &frac, this]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i++)
			{
				TargetInfo &info = _infoPairs[pairs[i]];
				if (info.close)
				{
					continue;
				}

				const int &p = info.p;
				const int &q = info.q;
				if (!filter_in(p) || !filter_in(q))
				{
					continue;
				}
				
				long double potential = lookup(p, q, -1);
				
				if (potential != potential)
				{
					continue;
				}

				long double ref_distance = info.target(frac);
				long double reference = lookup(p, q, ref_distance);

				long double diff = potential - reference;
				
				if (diff > 0 && std::isfinite(diff))
				{
					total += diff;
				}
			};
		};
		
		loop(check_clashes);
		
		return {(float)total, frac};
	};
	
	return new Task<BundleBonds *, ActivationEnergy>(job, "bundled clashes");
}

Task<BundleBonds *, Contacts> *
PairwiseDeviations::contact_map(const std::set<ScoreBucket> &forResidues)
{
	LoopMechanism loop = loop_mechanism(pairs(), perResiduePairs(), forResidues);

	auto job = [loop, this]
	(BundleBonds *bb) -> Contacts
	{
		auto lookup = bb->lookup();
		float frac = bb->frac();
		Contacts contacts{};

		auto check_clashes = [&lookup, &frac, &contacts, this]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i++)
			{
				TargetInfo &info = _infoPairs[pairs[i]];
				if (info.close)
				{
					continue;
				}

				const int &p = info.p;
				const int &q = info.q;
				
				long double potential = lookup(p, q, -1);
				
				if (potential != potential)
				{
					continue;
				}

				const ScoreBucket &l(_atoms[p]);
				const ScoreBucket &r(_atoms[q]);

				long double ref_distance = info.target(frac);
				long double reference = lookup(p, q, ref_distance);

				long double diff = potential - reference;
				
				if (diff > 0 && std::isfinite(diff))
				{
					contacts[l][r] += diff;
				}
			};
		};
		
		loop(check_clashes);
		
		return contacts;
	};
	
	return new Task<BundleBonds *, Contacts>(job, "contact map");
}


std::vector<int> &PairwiseDeviations::pairsForBlockIdx
(const int &bidx)
{
	return _neighbouring[bidx];
}
