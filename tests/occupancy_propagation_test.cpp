#include <doctest/doctest.h>
#include <vagabond/core/protonic/OccupancyPropagation.h>
#include <vector>

using namespace OccupancyPropagation;

namespace
{

// one subnetwork from explicit 0/1 state columns, one per atom (equal
// Boltzmann weight per state).
Subnetwork make(const std::vector<int> &ids,
                const std::vector<std::vector<int>> &cols)
{
	Subnetwork s;
	s.ids = ids;
	int n = (int)cols[0].size();
	s.X = Eigen::MatrixXf(n, (int)ids.size());
	s.p = Eigen::VectorXf::Ones(n);
	s.fraction = Eigen::VectorXf((int)ids.size());

	for (size_t j = 0; j < ids.size(); j++)
	{
		float sum = 0;
		for (int i = 0; i < n; i++)
		{
			s.X(i, (int)j) = (float)cols[j][i];
			sum += (float)cols[j][i];
		}
		s.fraction((int)j) = sum / (float)n;
	}

	return s;
}

// each atom's pooled occupancy: mass-weighted mean of its local fractions.
Eigen::VectorXf pooled(const std::vector<Subnetwork> &subs, int atoms)
{
	Eigen::VectorXf num = Eigen::VectorXf::Zero(atoms);
	Eigen::VectorXf den = Eigen::VectorXf::Zero(atoms);

	for (const Subnetwork &s : subs)
	{
		for (size_t j = 0; j < s.ids.size(); j++)
		{
			num(s.ids[j]) += s.mass * s.fraction((int)j);
			den(s.ids[j]) += s.mass;
		}
	}

	return num.cwiseQuotient(den);
}

const std::vector<int> trp80 = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
const std::vector<int> filler = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

// atom 0 (like His155) shares `shared` subnetworks of four perfectly
// (anti-)correlated states, 50/50, with atom 1 (like Trp156), which is
// 80/20 in 50 other subnetworks. Returns atom 0's corrected occupancy in the
// first shared subnetwork, and the reference's pooled global occupancy.
float run(int shared, bool anti, float &trpGlobal)
{
	std::vector<Subnetwork> subs;
	std::vector<int> his = anti ? std::vector<int>{0, 0, 1, 1}
	                            : std::vector<int>{1, 1, 0, 0};

	for (int i = 0; i < shared; i++)
	{
		subs.push_back(make({0, 1}, {his, {1, 1, 0, 0}}));
	}
	for (int i = 0; i < 50; i++)
	{
		subs.push_back(make({1, 2}, {trp80, filler}));
	}

	Eigen::VectorXf theta = pooled(subs, 3);
	trpGlobal = theta(1);
	return correct(subs, theta)[0](0);
}

}

TEST_CASE("propagation: a correlated, better-sampled atom drags a poorly "
          "sampled one towards it, but never past it")
{
	for (int shared : {5, 25})
	{
		float trp = 0;
		float his = run(shared, false, trp);

		CHECK(his > 0.5f);   // moved towards the reference
		CHECK(his < trp);    // and never beyond its global value
	}
}

TEST_CASE("propagation: a correlation from too few states is treated as "
          "noise")
{
	// one subnetwork of four states is perfectly correlated, but that is
	// only 2 standard errors (|r| * sqrt(4)), under Settings::significance
	// (3) - a correlation measured from a handful of states is not evidence
	float trp = 0;
	float his = run(1, false, trp);

	CHECK(his == doctest::Approx(0.5f));
}

TEST_CASE("propagation: more shared evidence trusts the correlation more")
{
	float trp1 = 0, trp25 = 0;
	float his1 = run(1, false, trp1);
	float his25 = run(25, false, trp25);

	// fraction of the gap between the poorly sampled atom's local 0.5 and
	// the reference's global value that was closed
	CHECK((his25 - 0.5f) / (trp25 - 0.5f) > (his1 - 0.5f) / (trp1 - 0.5f));
	// 25 subnetworks x 4 states = 100 effective states, shrunk by
	// 100 / (100 + 10) (Settings::shrinkStates) and by the correlation-matrix
	// repair - so close to, but deliberately a little short of, the whole gap
	CHECK((his25 - 0.5f) / (trp25 - 0.5f) > 0.85f);
}

TEST_CASE("propagation: anti-correlation mirrors correlation")
{
	float trp = 0;
	float same = run(25, false, trp);
	float anti = run(25, true, trp);

	CHECK(same + anti == doctest::Approx(1.f).epsilon(1e-3));
}

TEST_CASE("propagation: subnetworks already agreeing with the global "
          "values are left alone")
{
	std::vector<Subnetwork> subs;
	for (int i = 0; i < 50; i++)
	{
		subs.push_back(make({0, 1}, {trp80, trp80}));
	}

	Eigen::VectorXf theta = pooled(subs, 2);
	std::vector<Eigen::VectorXf> out = correct(subs, theta);

	CHECK(out[0](0) == doctest::Approx(0.8f));
	CHECK(out[0](1) == doctest::Approx(0.8f));
}

TEST_CASE("propagation: an uncorrelated reference does not move the target")
{
	std::vector<Subnetwork> subs;
	for (int i = 0; i < 25; i++)
	{
		subs.push_back(make({0, 1}, {{1, 0, 1, 0}, {1, 1, 0, 0}}));
	}
	for (int i = 0; i < 50; i++)
	{
		subs.push_back(make({1, 2}, {trp80, filler}));
	}

	Eigen::VectorXf theta = pooled(subs, 3);
	CHECK(correct(subs, theta)[0](0) == doctest::Approx(0.5f).epsilon(1e-3));
}

TEST_CASE("propagation: identical references share the credit rather than "
          "double it")
{
	std::vector<Subnetwork> single, doubled;

	for (int i = 0; i < 25; i++)
	{
		single.push_back(make({0, 1}, {{1, 1, 0, 0}, {1, 1, 0, 0}}));
		doubled.push_back(make({0, 1, 3},
		                       {{1, 1, 0, 0}, {1, 1, 0, 0}, {1, 1, 0, 0}}));
	}
	for (int i = 0; i < 50; i++)
	{
		single.push_back(make({1, 2}, {trp80, filler}));
		doubled.push_back(make({1, 3, 2}, {trp80, trp80, filler}));
	}

	float one = correct(single, pooled(single, 3))[0](0);
	float two = correct(doubled, pooled(doubled, 4))[0](0);

	CHECK(two == doctest::Approx(one).epsilon(0.05));
}

TEST_CASE("propagation: a small subnetwork with a mostly-unassigned "
          "reference still carries the reference's global value across")
{
	// atom 0 (like Met46) is in one subnetwork of 210 arrangements, shared
	// with atom 1 (like B110), which is only recorded in 76 of them (40
	// present, 36 absent - 19% and 17% of the arrangements; the rest are
	// unassigned) and is perfectly correlated with atom 0 wherever it is
	// recorded. Atom 1 is 8/25 = 32% present in 50 other subnetworks.
	Subnetwork shared;
	shared.ids = {0, 1};
	shared.X = Eigen::MatrixXf::Zero(210, 2);
	shared.recorded = Eigen::MatrixXf::Zero(210, 2);
	shared.p = Eigen::VectorXf::Ones(210);

	for (int r = 0; r < 210; r++)
	{
		shared.recorded(r, 0) = 1.f;
		if (r < 40)
		{
			shared.X(r, 0) = 1.f;
			shared.X(r, 1) = 1.f;
			shared.recorded(r, 1) = 1.f;
		}
		else if (r < 76)
		{
			shared.recorded(r, 1) = 1.f; // both absent
		}
		else
		{
			shared.X(r, 0) = (r % 2) ? 1.f : 0.f; // atom 1 unassigned here
		}
	}
	shared.fraction = Eigen::VectorXf(2);
	shared.fraction(0) = shared.X.col(0).mean();
	shared.fraction(1) = 40.f / 76.f;

	std::vector<int> b110_global(25, 0);
	for (int i = 0; i < 8; i++)
	{
		b110_global[i] = 1;
	}
	std::vector<int> other(25, 0);
	for (int i = 0; i < 25; i += 2)
	{
		other[i] = 1;
	}

	std::vector<Subnetwork> subs;
	subs.push_back(shared);
	for (int i = 0; i < 50; i++)
	{
		subs.push_back(make({1, 2}, {b110_global, other}));
	}

	Eigen::VectorXf theta = pooled(subs, 3);
	REQUIRE(theta(1) < 0.4f); // the reference's global value is ~0.32

	Propagator propagator;
	for (const Subnetwork &sub : subs)
	{
		propagator.accumulate(sub);
	}

	float before = shared.fraction(0);
	float after = propagator.correct(shared, theta)(0);

	// moved well over half of the way from its local value to the
	// reference's global one - the case where, with the correlation shrunk
	// by subnetwork mass and diluted by the unassigned states, it did not
	// move at all
	CHECK(after < before - 0.5f * (before - theta(1)));
}

TEST_CASE("propagation: a single-atom subnetwork passes through unchanged")
{
	std::vector<Subnetwork> subs;
	subs.push_back(make({0}, {{1, 0, 0, 0}}));

	Eigen::VectorXf theta(1);
	theta << 0.9f;

	CHECK(correct(subs, theta)[0](0) == doctest::Approx(0.25f));
}
