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
// unrepresentative theirs were too. correct() removes that
// unrepresentativeness using every other atom in the subnetwork at once, as
// a ridge-regularised multiple regression (see correct()), rather than
// picking any one of them.
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
		// for when pooling covariances across subnetworks.
		float mass = 1.f;

		// each atom's occupancy in this subnetwork alone: weighted
		// mean of its X column, as the caller already computed it.
		Eigen::VectorXf fraction;
	};

	struct Settings
	{
		// ridge penalty, as a fraction of each reference atom's own
		// variance. Pulls every regression slope towards zero, so that
		// atoms carrying the same information share the credit between
		// them (two identical references get about half each) and thin
		// data produces a small correction rather than a wild one.
		float ridge = 0.02f;

		// shrinks each pair's pooled covariance by W / (W + shrinkWeight),
		// where W is the pair's total shared weight across every
		// subnetwork containing both, in units of an average subnetwork's
		// mass. A pair co-sampled in only one or two subnetworks barely
		// counts; one co-sampled in many counts nearly in full.
		float shrinkWeight = 1.f;
	};

	/** @param theta every atom's pooled (global) occupancy, indexed by the
	 *  global atom index used in Subnetwork::ids.
	 *  @returns for each subnetwork, the corrected local fraction of each
	 *  of its atoms (same order as ids), clamped to [0, 1].
	 *
	 *  The correlation between two atoms is pooled (weighted by subnetwork
	 *  mass) over the *within-subnetwork* weighted correlation of every
	 *  subnetwork where both vary - correlations rather than covariances,
	 *  since an atom's variance differs between subnetworks, and pooling it
	 *  made the slopes overshoot. Each subnetwork then rebuilds covariances
	 *  from the pooled correlations and its own local standard deviations
	 *  (so two identical atoms get a slope of exactly 1), and each atom i is
	 *  regressed on all of the others at once (slopes beta_ij,
	 *  ridge-regularised), and corrected by:
	 *
	 *      f'_i = f_i + sum_j beta_ij * (theta_j - f_j)
	 *
	 *  - the standard control-variate/regression estimator, each other atom
	 *  j acting as a control whose true value (theta_j) is known better than
	 *  this subnetwork's own sample of it (f_j). An atom seen in only one
	 *  subnetwork has theta_j == f_j, so it contributes nothing. A
	 *  subnetwork whose atoms already agree with their global values is left
	 *  unchanged. */
	std::vector<Eigen::VectorXf>
	correct(const std::vector<Subnetwork> &subs, const Eigen::VectorXf &theta,
	        const Settings &settings = Settings());
}

#endif
