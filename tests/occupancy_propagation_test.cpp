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
	for (int shared : {1, 5, 25})
	{
		float trp = 0;
		float his = run(shared, false, trp);

		CHECK(his > 0.5f);   // moved towards the reference
		CHECK(his < trp);    // and never beyond its global value
	}
}

TEST_CASE("propagation: more shared evidence trusts the correlation more")
{
	float trp1 = 0, trp25 = 0;
	float his1 = run(1, false, trp1);
	float his25 = run(25, false, trp25);

	// fraction of the gap between the poorly sampled atom's local 0.5 and
	// the reference's global value that was closed
	CHECK((his25 - 0.5f) / (trp25 - 0.5f) > (his1 - 0.5f) / (trp1 - 0.5f));
	CHECK((his25 - 0.5f) / (trp25 - 0.5f) > 0.9f);
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

TEST_CASE("propagation: a single-atom subnetwork passes through unchanged")
{
	std::vector<Subnetwork> subs;
	subs.push_back(make({0}, {{1, 0, 0, 0}}));

	Eigen::VectorXf theta(1);
	theta << 0.9f;

	CHECK(correct(subs, theta)[0](0) == doctest::Approx(0.25f));
}
