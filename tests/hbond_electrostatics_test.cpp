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

// an idealised linear N-H...O=C contact: acceptor O at the origin, donor
// N 2.9 A away along +x, hydrogen on the line between them at H...O = 1.9 A,
// and the acceptor's parent carbon 1.23 A from O at the given angle to the
// O->donor direction.
struct Contact
{
	glm::vec3 acceptor{0.f, 0.f, 0.f};
	glm::vec3 donor{2.9f, 0.f, 0.f};
	glm::vec3 hydrogen{1.9f, 0.f, 0.f};

	glm::vec3 parent(float angleDegrees) const
	{
		float t = angleDegrees * 3.14159265f / 180.f;
		return {1.23f * std::cos(t), 1.23f * std::sin(t), 0.f};
	}
};

}

TEST_CASE("hbond electrostatics: with no parents it is donor-acceptor "
          "repulsion minus hydrogen-acceptor attraction")
{
	Contact c;
	float sum = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor, {});

	CHECK(sum == doctest::Approx(inv(c.donor, c.acceptor) -
	                             inv(c.hydrogen, c.acceptor)));
	CHECK(sum < 0.f); // the hydrogen is closer, so net attraction
}

TEST_CASE("hbond electrostatics: each parent term has the sign of the "
          "product of its charges")
{
	Contact c;
	glm::vec3 p = c.parent(150.f);
	float without = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor, {});
	float with = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                      {Parent{p, 1.f}});

	// hydrogen (+) with parent (+): repulsion; donor (-) with parent (+):
	// attraction - exactly these two, added to the no-parent sum
	CHECK(with - without == doctest::Approx(inv(c.hydrogen, p) -
	                                        inv(c.donor, p)));

	// the parent is closer to the hydrogen than to the donor, so its
	// repulsion of the hydrogen outweighs its attraction of the donor and it
	// weakens the net attraction - the two dipoles partly cancelling
	CHECK(with > without);
}

TEST_CASE("hbond electrostatics: the +delta shared among parents adds up "
          "the same however many parents it is split between")
{
	Contact c;
	glm::vec3 p = c.parent(150.f);

	float one = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                     {Parent{p, 1.f}});

	// the same parent position counted as two halves
	float two = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                     {Parent{p, 0.5f}, Parent{p, 0.5f}});

	CHECK(two == doctest::Approx(one));

	// two different parents each carry half, so each matters half as much
	// as a lone parent in that place would
	glm::vec3 q = c.parent(120.f);
	float split = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                       {Parent{p, 0.5f}, Parent{q, 0.5f}});
	float lone_p = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                        {Parent{p, 1.f}});
	float lone_q = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                        {Parent{q, 1.f}});
	CHECK(split == doctest::Approx(0.5f * (lone_p + lone_q)));
}

TEST_CASE("hbond electrostatics: at an ideal contact it is about as strong "
          "as the hydrogen-acceptor term it replaced")
{
	Contact c;
	float sum = Energy::hbond_dipole_sum(c.donor, c.hydrogen, c.acceptor,
	                                     {Parent{c.parent(150.f), 1.f}});
	float energy = Energy::HBondElectrostatic * Energy::HBondDeltaCharge *
	Energy::HBondDeltaCharge * sum;

	float old_term = -0.5f / (1.9f * 1.9f); // about -0.14

	CHECK(energy < 0.f);
	CHECK(std::fabs(energy - old_term) < 0.03f);
}

TEST_CASE("hbond electrostatics: coincident points cannot blow the sum up")
{
	glm::vec3 p{0.f, 0.f, 0.f};
	float sum = Energy::hbond_dipole_sum(p, p, p, {Parent{p, 1.f}});

	CHECK(std::isfinite(sum));
}
