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

#ifndef __vagabond__OccupancyPropagation__
#define __vagabond__OccupancyPropagation__

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <vagabond/utils/Eigen/Dense>

// Pure numerics (no Clique/Probe/GUI dependency) for propagating
// occupancy information between atoms - see OccupancyPredictor, which
// gathers the inputs and decides whether to apply this.
//
// An atom that only turns up in a few subnetworks (or only near their
// edges) gets a poor occupancy estimate from those subnetworks alone. But
// it is usually correlated with other atoms that were sampled much more
// thoroughly elsewhere. Within one subnetwork, how far each atom's own
// occupancy there (its "local" fraction) sits from that atom's pooled
// "global" occupancy tells us how unrepresentative the subnetwork's states
// were of the whole network - and, for any atoms correlated with it, how
// unrepresentative theirs were too. Propagator removes that
// unrepresentativeness using every correlated atom in the subnetwork at
// once, as a regularised multiple regression, rather than picking any one.
namespace OccupancyPropagation
{
	// one subnetwork's worth of binary (present/absent) atom states.
	struct Subnetwork
	{
		// global atom index of each column of X.
		std::vector<int> ids;

		// states x ids.size(); 1 = present, 0 = absent. An atom not
		// recorded in some state is imputed by the caller with that atom's
		// own local fraction (so it contributes no variance or covariance).
		Eigen::MatrixXf X;

		// one non-negative Boltzmann weight per state (any scale - it is
		// normalised here).
		Eigen::VectorXf p;

		// this subnetwork's total weight, in any consistent unit across
		// subnetworks (its total Boltzmann mass, say) - what it counts
		// for when pooling correlations across subnetworks.
		float mass = 1.f;

		// each atom's occupancy in this subnetwork alone: weighted
		// mean of its X column, as the caller already computed it.
		Eigen::VectorXf fraction;
	};

	struct Settings
	{
		// ridge penalty, as a fraction of each atom's own variance.
		float ridge = 0.02f;

		// shrinks each pair's pooled correlation by W / (W + shrinkWeight),
		// where W is the pair's total shared weight across every
		// subnetwork where both vary, in units of an average subnetwork's
		// mass. A pair co-sampled in only one or two subnetworks barely
		// counts; one co-sampled in many counts nearly in full.
		float shrinkWeight = 1.f;

		// pooled correlations weaker than this are treated as zero. Many
		// tiny, noisy correlations added together would otherwise add up to
		// a large spurious correction, and zeroing them also splits each
		// subnetwork into small independent groups of related atoms, which
		// is what keeps the cost down.
		float minCorrelation = 0.1f;

		// the pooled correlation matrix is stitched together pair by pair
		// from different subsets of subnetworks, so it needn't be a valid
		// (positive semi-definite) matrix, and inverting a near-singular
		// one makes the regression wildly overshoot. Its eigenvalues are
		// raised to at least this (the matrix has a unit diagonal) before
		// use - the "nearest valid correlation matrix" repair.
		float eigenFloor = 0.03f;
	};

	// what happened to one atom in one subnetwork, to explain a correction
	// that is small or absent.
	struct Diagnostic
	{
		// strongest pooled correlation with any other atom here (before
		// minCorrelation is applied).
		float maxCorrelation = 0;

		// largest |theta_j - f_j| over the atoms it is correlated with
		// (|correlation| >= minCorrelation): how much those atoms' local
		// values here disagree with their global ones.
		float maxDeviation = 0;

		// correlated atoms that disagree noticeably (> 0.02).
		int usefulReferences = 0;

		// the correction was limited to the largest reference deviation.
		bool capped = false;

		// the (up to three) other atoms that moved this one most:
		// `index` is the position in Subnetwork::ids, `correlation` the
		// pooled correlation, `slope` the regression slope beta, `deviation`
		// theta_j - f_j; the contribution to the correction is slope *
		// deviation.
		struct Reference
		{
			int index = 0;
			float correlation = 0;
			float slope = 0;
			float deviation = 0;
		};
		std::vector<Reference> topReferences;
	};

	/** Two passes over the subnetworks, one at a time, so only one
	 *  subnetwork's states need exist in memory at any moment:
	 *
	 *  1. accumulate() every subnetwork - pools the within-subnetwork
	 *     weighted *correlation* of every pair of atoms over the
	 *     subnetworks where both vary (correlations, not covariances: an
	 *     atom's variance differs between subnetworks, and pooling it made
	 *     the regression slopes overshoot).
	 *  2. correct() each subnetwork (again, one at a time).
	 *
	 *  correct() rebuilds covariances from the pooled correlations and that
	 *  subnetwork's own local standard deviations (so two identical atoms
	 *  get a slope of exactly 1), splits the atoms into groups connected by
	 *  a correlation of at least minCorrelation, repairs each group's
	 *  correlation matrix (eigenFloor), regresses each atom on all the
	 *  others in its group at once, and corrects it:
	 *
	 *      f'_i = f_i + sum_j beta_ij * (theta_j - f_j)
	 *
	 *  - the control-variate/regression estimator, each other atom j acting
	 *  as a control whose true value (theta_j) is known better than this
	 *  subnetwork's own sample of it (f_j). An atom seen in only one
	 *  subnetwork has theta_j == f_j, so it contributes nothing. The
	 *  correction is never larger than the largest deviation among the
	 *  atoms it is correlated with, and the result is clamped to [0, 1]. */
	class Propagator
	{
	public:
		Propagator(const Settings &settings = Settings());

		void accumulate(const Subnetwork &sub);

		/** @param theta every atom's pooled (global) occupancy, indexed by
		 *  the global atom index used in Subnetwork::ids.
		 *  @returns the corrected local fraction of each atom, in the order
		 *  of sub.ids. */
		Eigen::VectorXf correct(const Subnetwork &sub,
		                        const Eigen::VectorXf &theta,
		                        std::vector<Diagnostic> *diagnostics = nullptr)
		const;

	private:
		struct PairStats
		{
			double corr = 0; // sum of mass * weighted correlation
			double weight = 0; // sum of mass, where the pair both varied
		};

		float correlation(int a, int b) const;

		Settings _settings;
		std::unordered_map<uint64_t, PairStats> _pooled;
		double _totalMass = 0;
		size_t _count = 0;
	};

	/** convenience: both passes over an in-memory list (used by the tests;
	 *  the predictor streams instead, to avoid holding every subnetwork). */
	std::vector<Eigen::VectorXf>
	correct(const std::vector<Subnetwork> &subs, const Eigen::VectorXf &theta,
	        const Settings &settings = Settings());
}

#endif
