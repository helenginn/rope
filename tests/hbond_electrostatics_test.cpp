#include <doctest/doctest.h>
#include <vagabond/core/protonic/Energy.h>
#include <cmath>
#include <vector>

using hnet::Energy;
typedef Energy::ChargedPosition Parent;

namespace
{

// 1 / distance, the same unit every term is in
float inv(const glm::vec3 &a, const glm::vec3 &b)
{
	return 1.f / glm::length(a - b);
}

// An idealised linear amide N-H...O=C contact: the acceptor O at the origin,
// the donor N 2.9 A away along +x, and the donor's hydrogen on the line
// between them, 1.0 A from N (1.9 A from O). O's parent carbon is 1.23 A from
// O at 150 degrees to the O->N direction; N's parents (C and C-alpha) are
// about 120 degrees round from the N-H direction, 1.33 A and 1.46 A away.
struct Contact
{
	glm::vec3 acceptor{0.f, 0.f, 0.f};
	glm::vec3 donor{2.9f, 0.f, 0.f};
	glm::vec3 hydrogen{1.9f, 0.f, 0.f};

	glm::vec3 acceptorParent() const
	{
		float t = 150.f * 3.14159265f / 180.f;
		return {1.23f * std::cos(t), 1.23f * std::sin(t), 0.f};
	}

	std::vector<Parent> donorParents() const
	{
		return {Parent{{2.9f + 1.33f * 0.5f, 1.33f * 0.866f, 0.f}, 0.5f},
		        Parent{{2.9f + 1.46f * 0.5f, -1.46f * 0.866f, 0.f}, 0.5f}};
	}
};

float sum(const glm::vec3 &donor, const glm::vec3 &hydrogen,
          const glm::vec3 &acceptor, const std::vector<Parent> &parents,
          float fD = 1.f, float fH = 1.f, float fA = 1.f)
{
	return Energy::hbond_electrostatic_sum(donor, hydrogen, acceptor, parents,
	                                       fD, fH, fA);
}

}

TEST_CASE("hbond electrostatics: with no hydrogen the donor and acceptor "
          "still repel")
{
	Contact c;

	// no hydrogen at all: only the two atoms' own charges are left
	float without = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f);

	// half of the donor-acceptor repulsion - the mirror-image half-bond
	// carries the other half
	CHECK(without == doctest::Approx(0.5f * inv(c.donor, c.acceptor)));
	CHECK(without > 0.f);
}

TEST_CASE("hbond electrostatics: a hydrogen on the donor adds attraction "
          "to the acceptor")
{
	Contact c;
	float without = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f);
	float with = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 1.f, 1.f);

	CHECK(with - without == doctest::Approx(-inv(c.hydrogen, c.acceptor)));
	CHECK(with < without);
}

TEST_CASE("hbond electrostatics: parent terms have the sign of the product "
          "of their charges")
{
	Contact c;
	glm::vec3 p = c.acceptorParent();

	// donor (-) with parent (+): attraction, with or without the hydrogen
	float no_h = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f);
	float no_h_parent = sum(c.donor, c.hydrogen, c.acceptor, {Parent{p, 1.f}},
	                        1.f, 0.f, 1.f);
	CHECK(no_h_parent - no_h == doctest::Approx(-inv(c.donor, p)));

	// hydrogen (+) with parent (+): repulsion, on top of that
	float with_h_parent = sum(c.donor, c.hydrogen, c.acceptor,
	                          {Parent{p, 1.f}});
	float with_h = sum(c.donor, c.hydrogen, c.acceptor, {});
	CHECK(with_h_parent - with_h ==
	      doctest::Approx(inv(c.hydrogen, p) - inv(c.donor, p)));
}

TEST_CASE("hbond electrostatics: the +delta shared among parents adds up "
          "the same however many parents it is split between")
{
	Contact c;
	glm::vec3 p = c.acceptorParent();

	float one = sum(c.donor, c.hydrogen, c.acceptor, {Parent{p, 1.f}});

	// the same position counted as two halves
	float two = sum(c.donor, c.hydrogen, c.acceptor,
	                {Parent{p, 0.5f}, Parent{p, 0.5f}});
	CHECK(two == doctest::Approx(one));

	// two different parents each carry half
	glm::vec3 q{-0.6f, 1.07f, 0.f};
	float split = sum(c.donor, c.hydrogen, c.acceptor,
	                  {Parent{p, 0.5f}, Parent{q, 0.5f}});
	float lone_p = sum(c.donor, c.hydrogen, c.acceptor, {Parent{p, 1.f}});
	float lone_q = sum(c.donor, c.hydrogen, c.acceptor, {Parent{q, 1.f}});

	// the donor-acceptor term is common to all three, so take it out
	float common = sum(c.donor, c.hydrogen, c.acceptor, {});
	CHECK(split - common ==
	      doctest::Approx(0.5f * (lone_p - common) + 0.5f * (lone_q - common)));
}

TEST_CASE("hbond electrostatics: a hydrogen nearer the acceptor is its own "
          "hydrogen and has no electrostatic interaction with it")
{
	Contact c;
	glm::vec3 p = c.acceptorParent();

	// the same hydrogen, but 1.0 A from the ACCEPTOR instead (bonded to it)
	glm::vec3 own{1.0f, 0.f, 0.f};
	float hydrogen_bonded_to_acceptor =
	sum(c.donor, own, c.acceptor, {Parent{p, 1.f}});
	float no_hydrogen =
	sum(c.donor, own, c.acceptor, {Parent{p, 1.f}}, 1.f, 0.f, 1.f);

	CHECK(hydrogen_bonded_to_acceptor == doctest::Approx(no_hydrogen));

	// and one exactly midway hasn't been placed on either atom
	glm::vec3 midway = 0.5f * (c.donor + c.acceptor);
	CHECK(sum(c.donor, midway, c.acceptor, {}, 1.f, 1.f, 1.f) ==
	      doctest::Approx(sum(c.donor, midway, c.acceptor, {}, 1.f, 0.f, 1.f)));
}

TEST_CASE("hbond electrostatics: a hydrogen must be clearly nearer the donor "
          "to count as its own")
{
	Contact c;

	// 0.1 A nearer the donor than midway: nearer, but inside the margin, so
	// not distinguishable from unplaced (and not at the mercy of rounding)
	glm::vec3 midway = 0.5f * (c.donor + c.acceptor);
	glm::vec3 almost = midway + glm::vec3{0.05f, 0.f, 0.f};
	CHECK(sum(c.donor, almost, c.acceptor, {}, 1.f, 1.f, 1.f) ==
	      doctest::Approx(sum(c.donor, almost, c.acceptor, {}, 1.f, 0.f, 1.f)));

	// clearly the donor's - about 1 A from it, 1.9 from the acceptor - counts
	CHECK(sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 1.f, 1.f) <
	      sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f));
}

TEST_CASE("hbond electrostatics: absent atoms switch off their own terms")
{
	Contact c;
	glm::vec3 p = c.acceptorParent();

	// no acceptor: no acceptor, no parents' charge, nothing to repel
	CHECK(sum(c.donor, c.hydrogen, c.acceptor, {Parent{p, 1.f}}, 1.f, 1.f,
	          0.f) == doctest::Approx(0.f));

	// no donor: its own terms go, but its hydrogen (if there) still counts
	float no_donor = sum(c.donor, c.hydrogen, c.acceptor, {Parent{p, 1.f}},
	                     0.f, 1.f, 1.f);
	CHECK(no_donor == doctest::Approx(-inv(c.hydrogen, c.acceptor) +
	                                  inv(c.hydrogen, p)));

	// half-settled existence scales a term by a half
	float full = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f);
	float half = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 0.5f);
	CHECK(half == doctest::Approx(0.5f * full));
}

TEST_CASE("hbond electrostatics: the two half-bonds of a pair carry the "
          "donor-acceptor repulsion between them exactly once")
{
	Contact c;

	// without a hydrogen or parents, each half-bond (roles swapped) carries
	// half of it
	float a = sum(c.donor, c.hydrogen, c.acceptor, {}, 1.f, 0.f, 1.f);
	float b = sum(c.acceptor, c.hydrogen, c.donor, {}, 1.f, 0.f, 1.f);

	CHECK(a + b == doctest::Approx(inv(c.donor, c.acceptor)));
}

TEST_CASE("hbond electrostatics: forming the H-bond is worth about as much "
          "as the hydrogen-acceptor term it replaced")
{
	Contact c;

	// the two half-bonds of the pair: the O half (O as acceptor, N donating)
	// and the N half (N as acceptor, O donating - the hydrogen is N's own, so
	// only the atoms' charges count there)
	auto both = [&c](float fH)
	{
		return sum(c.donor, c.hydrogen, c.acceptor, {Parent{c.acceptorParent(), 1.f}},
		           1.f, fH, 1.f) +
		sum(c.acceptor, c.hydrogen, c.donor, c.donorParents(), 1.f, fH, 1.f);
	};

	float scale = Energy::HBondElectrostatic * Energy::HBondDeltaCharge *
	Energy::HBondDeltaCharge;

	// what decides the state weights is the difference between the two states
	float formed = scale * (both(1.f) - both(0.f));

	CHECK(formed < 0.f);
	CHECK(std::fabs(formed - (-0.14f)) < 0.03f);
}

namespace
{

// a donor-hydrogen...acceptor chain bent `bend` degrees out of line at the
// hydrogen: donor at the origin, hydrogen 1 A along +x, acceptor 1.9 A from
// the hydrogen in a direction turned `bend` degrees from straight on (so the
// D-H...A angle is 180 - bend)
float acceptance_at_bend(float bend, bool out_of_plane = false)
{
	float t = bend * 3.14159265f / 180.f;
	glm::vec3 donor{0.f, 0.f, 0.f};
	glm::vec3 hydrogen{1.f, 0.f, 0.f};
	glm::vec3 turn = out_of_plane ? glm::vec3{std::cos(t), 0.f, std::sin(t)}
	                              : glm::vec3{std::cos(t), std::sin(t), 0.f};
	glm::vec3 acceptor = hydrogen + 1.9f * turn;

	return Energy::hbond_angle_acceptance(donor, hydrogen, acceptor);
}

}

TEST_CASE("hbond angle acceptance: fully on at a collinear geometry, never "
          "NaN")
{
	// hydrogen on the line from the donor to the acceptor, in many
	// orientations: rounding used to push the dot product a hair over 1
	// (acos -> NaN, silently dropped) in roughly one case in five
	int bad = 0;
	for (int i = 1; i <= 2000; i++)
	{
		float s = (float)i;
		glm::vec3 acceptor{std::sin(s) * 7.f, std::cos(s * 1.3f) * 5.f,
		                   std::sin(s * 0.7f) * 9.f};
		glm::vec3 donor{std::cos(s * 0.9f) * 6.f, std::sin(s * 1.7f) * 8.f,
		                std::cos(s * 0.4f) * 4.f};
		glm::vec3 hydrogen = 0.5f * (acceptor + donor);

		float a = Energy::hbond_angle_acceptance(donor, hydrogen, acceptor);
		if (!std::isfinite(a) || std::fabs(a - 1.f) > 1e-3f)
		{
			bad++;
		}
	}

	CHECK(bad == 0);
}

TEST_CASE("hbond angle acceptance: all stabilisation is lost at 30 degrees "
          "out of line")
{
	CHECK(acceptance_at_bend(0.f) == doctest::Approx(1.f));

	// a D-H...A angle of 150 degrees: exactly at the cutoff
	CHECK(acceptance_at_bend(30.f) == doctest::Approx(0.f).epsilon(1e-3));

	// and nothing beyond it, however far round
	for (float bend : {31.f, 45.f, 60.f, 90.f, 135.f, 180.f})
	{
		CHECK(acceptance_at_bend(bend) == 0.f);
	}
}

TEST_CASE("hbond angle acceptance: falls smoothly between linear and the "
          "cutoff")
{
	// cos(90 degrees * bend / 30): a half-way bend of 15 degrees is cos 45
	CHECK(acceptance_at_bend(15.f) == doctest::Approx(0.7071f).epsilon(1e-3));
	CHECK(acceptance_at_bend(10.f) == doctest::Approx(0.8660f).epsilon(1e-3));

	// strictly decreasing from 0 to 30 degrees
	float previous = 2.f;
	for (float bend = 0.f; bend < 30.f; bend += 2.5f)
	{
		float a = acceptance_at_bend(bend);
		CHECK(a < previous);
		CHECK(a >= 0.f);
		CHECK(a <= 1.0001f);
		previous = a;
	}
}

TEST_CASE("hbond angle acceptance: depends only on the size of the bend, "
          "not its direction")
{
	for (float bend : {5.f, 12.f, 20.f, 28.f})
	{
		CHECK(acceptance_at_bend(bend, true) ==
		      doctest::Approx(acceptance_at_bend(bend, false)));
		CHECK(acceptance_at_bend(-bend) ==
		      doctest::Approx(acceptance_at_bend(bend)));
	}
}

TEST_CASE("hbond angle acceptance: a hydrogen on top of an atom has none")
{
	glm::vec3 donor{0.f, 0.f, 0.f};
	glm::vec3 acceptor{2.9f, 0.f, 0.f};

	float on_donor = Energy::hbond_angle_acceptance(donor, donor, acceptor);
	float on_acceptor = Energy::hbond_angle_acceptance(donor, acceptor, acceptor);

	CHECK(std::isfinite(on_donor));
	CHECK(on_donor == 0.f);
	CHECK(std::isfinite(on_acceptor));
	CHECK(on_acceptor == 0.f);
}

TEST_CASE("hbond electrostatics: coincident points cannot blow the sum up")
{
	glm::vec3 p{0.f, 0.f, 0.f};
	float s = sum(p, p, p, {Parent{p, 1.f}});

	CHECK(std::isfinite(s));
}
