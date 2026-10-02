#include <doctest/doctest.h>
#include <vagabond/core/protonic/Energy.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

using hnet::Energy;
using hnet::EnergyTally;
using hnet::GetEnergy;

namespace
{

constexpr size_t N = Energy::SourceCount;

// the per-source flag/amplifier storage an Energy would own - modulate()
// points its baked terms at these, and the tally must keep reading them
struct Storage
{
	bool on[N];
	float amp[N];

	Storage()
	{
		for (size_t s = 0; s < N; s++)
		{
			on[s] = true;
			amp[s] = 1.f;
		}
	}
};

// one baked term set, as modulate() would make it
GetEnergy baked(Storage &st, std::initializer_list<std::pair<Energy::Source, float>>
                terms)
{
	Energy::BakedEnergy b;
	for (const auto &t : terms)
	{
		b.terms[b.count++] = {&st.on[t.first], &st.amp[t.first], t.second,
		                      t.first};
	}
	return GetEnergy(b);
}

// the replay the tally replaces: call every job
float replay(const std::vector<GetEnergy> &jobs)
{
	float total = 0;
	for (const GetEnergy &job : jobs)
	{
		total += job();
	}
	return total;
}

// a lot of random baked terms across every source
std::vector<GetEnergy> random_jobs(Storage &st, int count, unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> energy(-2.f, 2.f);

	std::vector<GetEnergy> jobs;
	for (int i = 0; i < count; i++)
	{
		auto pick = [&rng]() { return (Energy::Source)(rng() % N); };

		if (rng() % 2)
		{
			jobs.push_back(baked(st, {{pick(), energy(rng)}}));
		}
		else
		{
			jobs.push_back(baked(st, {{pick(), energy(rng)},
			                          {pick(), energy(rng)}}));
		}
	}
	return jobs;
}

}

TEST_CASE("energy tally: folding baked terms gives the same score as "
          "replaying each one")
{
	Storage st;
	std::vector<GetEnergy> jobs = random_jobs(st, 500, 1);

	EnergyTally tally;
	for (const GetEnergy &job : jobs)
	{
		tally.add(job);
	}

	CHECK(tally.liveCount() == 0); // all recognised as baked
	CHECK(tally.evaluate() == doctest::Approx(replay(jobs)).epsilon(1e-4));
	CHECK(tally.score()() == doctest::Approx(replay(jobs)).epsilon(1e-4));
}

TEST_CASE("energy tally: switching a source off or moving an amplifier after "
          "scoring still changes the score, as before")
{
	Storage st;
	std::vector<GetEnergy> jobs = random_jobs(st, 300, 2);

	EnergyTally tally;
	for (const GetEnergy &job : jobs)
	{
		tally.add(job);
	}
	std::function<float()> score = tally.score();

	// an energy-source tickbox
	st.on[Energy::Angle] = false;
	st.on[Energy::Torsion] = false;
	CHECK(score() == doctest::Approx(replay(jobs)).epsilon(1e-4));

	// a slider
	st.amp[Energy::Distance] = 2.7f;
	st.amp[Energy::Acceptor] = 0.4f;
	CHECK(score() == doctest::Approx(replay(jobs)).epsilon(1e-4));

	// everything off: nothing left
	for (size_t s = 0; s < N; s++)
	{
		st.on[s] = false;
	}
	CHECK(score() == doctest::Approx(0.f));
}

TEST_CASE("energy tally: a term that is not baked is kept and evaluated "
          "afresh each time (protonation reads the pH live)")
{
	Storage st;
	EnergyTally tally;
	tally.add(baked(st, {{Energy::Torsion, 1.5f}}));

	float live_value = 4.f;
	tally.add(GetEnergy([&live_value]() { return live_value; }));

	CHECK(tally.liveCount() == 1);

	std::function<float()> score = tally.score();
	CHECK(score() == doctest::Approx(1.5f + 4.f));

	live_value = -1.f; // the pH moved
	CHECK(score() == doctest::Approx(1.5f - 1.f));
}

TEST_CASE("energy tally: ignores empty terms, and an empty tally scores zero")
{
	EnergyTally tally;
	CHECK(tally.empty());

	tally.add(GetEnergy{}); // modulate() returns {} when nothing contributed
	CHECK(tally.empty());
	CHECK(tally.evaluate() == 0.f);
	CHECK(tally.score()() == 0.f);

	Storage st;
	tally.add(baked(st, {{Energy::Bulk, 1.f}}));
	CHECK(!tally.empty());
}

TEST_CASE("energy tally: the score outlives the tally that made it")
{
	Storage st;
	std::function<float()> score;
	float expected = 0;

	{
		EnergyTally tally;
		tally.add(baked(st, {{Energy::Acceptor, -1.f}, {Energy::Distance, -0.3f}}));
		tally.add(baked(st, {{Energy::Repulsion, 0.5f}}));
		expected = tally.evaluate();
		score = tally.score();
	}

	CHECK(score() == doctest::Approx(expected));
	CHECK(score() == doctest::Approx(-1.f - 0.3f + 0.5f));
}

TEST_CASE("protonation term: only a deprotonated state keeps a live term")
{
	Energy energy;
	energy.alter_source(Energy::Protonation, true);

	float delta_g = 3.f;
	auto get_delta_g = [&delta_g]() { return delta_g; };

	hnet::ExistenceConnector exist;
	hnet::CountConnector charge;
	GuiltVersion gv = 1;

	REQUIRE(exist.assign_value_and_check(hnet::Existence::Present, gv));

	auto wrapper = energy.energy_wrapper_for_protonation(
	charge, exist, hnet::Count::mOne, get_delta_g);

	// charge is deprotonated in this state: a live term that reads the
	// pH-dependent energy afresh each time it is replayed
	REQUIRE(charge.assign_value_and_check(hnet::Count::mOne, gv));
	GetEnergy deprotonated = wrapper(gv);
	REQUIRE(static_cast<bool>(deprotonated));
	CHECK(deprotonated() == doctest::Approx(3.f));

	delta_g = 7.f; // the pH moved
	CHECK(deprotonated() == doctest::Approx(7.f));

	// switching the source off silences it, live
	energy.alter_source(Energy::Protonation, false);
	CHECK(deprotonated() == doctest::Approx(0.f));
	energy.alter_source(Energy::Protonation, true);

	// folded into a tally it stays live, alongside the baked terms
	EnergyTally tally;
	tally.add(deprotonated);
	CHECK(tally.liveCount() == 1);
	CHECK(tally.evaluate() == doctest::Approx(7.f));
}

TEST_CASE("protonation term: a state that is not deprotonated, or absent, "
          "has no term at all")
{
	Energy energy;
	energy.alter_source(Energy::Protonation, true);
	auto get_delta_g = []() { return 3.f; };
	GuiltVersion gv = 1;

	// present, but protonated (not the deprotonated charge)
	{
		hnet::ExistenceConnector exist;
		hnet::CountConnector charge;
		REQUIRE(exist.assign_value_and_check(hnet::Existence::Present, gv));
		REQUIRE(charge.assign_value_and_check(hnet::Count::Zero, gv));

		auto wrapper = energy.energy_wrapper_for_protonation(
		charge, exist, hnet::Count::mOne, get_delta_g);

		CHECK(!static_cast<bool>(wrapper(gv)));
	}

	// absent altogether
	{
		hnet::ExistenceConnector exist;
		hnet::CountConnector charge;
		REQUIRE(exist.assign_value_and_check(hnet::Existence::Absent, gv));

		auto wrapper = energy.energy_wrapper_for_protonation(
		charge, exist, hnet::Count::mOne, get_delta_g);

		CHECK(!static_cast<bool>(wrapper(gv)));
	}

	// and an empty tally entry changes nothing
	EnergyTally tally;
	tally.add(GetEnergy{});
	CHECK(tally.empty());
}

TEST_CASE("energy tally: a state's score is far smaller and quicker to read "
          "back than keeping every term")
{
	Storage st;
	const int terms = 4000;   // energy terms in the wider clique
	const int states = 1000;  // accepted configurations

	std::vector<std::function<float()>> tallied;
	std::vector<std::vector<GetEnergy>> kept;

	for (int s = 0; s < states; s++)
	{
		std::vector<GetEnergy> jobs = random_jobs(st, terms, 100 + s);

		EnergyTally tally;
		for (const GetEnergy &job : jobs)
		{
			tally.add(job);
		}

		tallied.push_back(tally.score());
		if (s < 20)
		{
			kept.push_back(std::move(jobs)); // enough to time the old way
		}
	}

	auto time = [](auto &&fn)
	{
		auto start = std::chrono::steady_clock::now();
		float total = fn();
		auto end = std::chrono::steady_clock::now();
		return std::make_pair(
		std::chrono::duration<double, std::micro>(end - start).count(), total);
	};

	// reading 20 states' scores back, the old way (replay every term) and the
	// new way (per-source sums)
	auto old_way = time([&]()
	{
		float total = 0;
		for (const auto &jobs : kept) total += replay(jobs);
		return total;
	});
	auto new_way = time([&]()
	{
		float total = 0;
		for (int s = 0; s < 20; s++) total += tallied[s]();
		return total;
	});

	std::cout << "energy tally: " << terms << " terms per state - replaying 20 "
	          << "states took " << old_way.first << " us, the tally "
	          << new_way.first << " us ("
	          << old_way.first / std::max(new_way.first, 0.01) << "x)"
	          << std::endl;

	// the speed is reported, not asserted (timing is machine-dependent); what
	// must hold is that the answers agree
	CHECK(new_way.second == doctest::Approx(old_way.second).epsilon(1e-3));
}
