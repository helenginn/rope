#include <doctest/doctest.h>
#include <vagabond/core/protonic/Coordinated.h>
#include <cmath>

using hnet::Coordinated;

namespace
{

float deg(float radians)
{
	return radians * 180.f / 3.14159265f;
}

// the torsion grandparent-parent-me-child, folded into (-90, 90] the way
// the old planarity test did (a child in the plane on either side counts)
float folded_torsion(const glm::vec3 &p0, const glm::vec3 &p1,
                     const glm::vec3 &p2, const glm::vec3 &p3)
{
	glm::vec3 b0 = p0 - p1;
	glm::vec3 b1 = glm::normalize(p2 - p1);
	glm::vec3 b2 = p3 - p2;
	glm::vec3 v = b0 - b1 * glm::dot(b0, b1);
	glm::vec3 w = b2 - b1 * glm::dot(b2, b1);
	float x = glm::dot(v, w);
	float y = glm::dot(glm::cross(b1, v), w);
	float t = deg(std::atan2(y, x));

	while (t >= 90.f) t -= 180.f;
	while (t <= -90.f) t += 180.f;
	return t;
}

}

TEST_CASE("planar acceptance: an end-on donor is judged by its height above "
          "the plane, not by an exaggerated torsion (LEU247:O,B)")
{
	// from s157a_apo2.pdb: ASP134:N donating to the carbonyl O of LEU247 in
	// each conformer; TRP248:N is the carbonyl's planar reference
	glm::vec3 donor{-3.363f, -2.559f, 12.154f};
	glm::vec3 nextN{-5.643f, 1.222f, 11.120f};

	glm::vec3 oA{-4.203f, -0.413f, 10.567f}, cA{-4.732f, 0.659f, 10.313f};
	glm::vec3 oB{-3.729f, -0.002f, 10.983f}, cB{-4.561f, 0.810f, 10.465f};

	glm::vec3 midA = 0.5f * (donor + oA);
	glm::vec3 midB = 0.5f * (donor + oB);

	float elevationA = Coordinated::outOfPlaneAngle(nextN, cA, oA, midA);
	float elevationB = Coordinated::outOfPlaneAngle(nextN, cB, oB, midB);
	float torsionB = std::fabs(folded_torsion(nextN, cB, oB, midB));

	// both are comfortably inside the 45 degree limit...
	CHECK(elevationA < 12.f);
	CHECK(elevationB > 24.f);
	CHECK(elevationB < 32.f);

	// ...where the torsion put B at 57 degrees, past it - which is why the
	// B conformer was never offered as a candidate
	CHECK(torsionB > 45.f);
}

TEST_CASE("planar acceptance: at right angles to the bond, elevation and "
          "torsion agree")
{
	// grandparent, parent and me in the xy plane; the child at right angles
	// to the parent-me bond (x axis) and 30 degrees round it out of the plane
	glm::vec3 grandparent{-1.f, 1.f, 0.f};
	glm::vec3 parent{0.f, 0.f, 0.f};
	glm::vec3 me{1.5f, 0.f, 0.f};

	float t = 30.f * 3.14159265f / 180.f;
	glm::vec3 child = me + glm::vec3{0.f, -std::cos(t), std::sin(t)};

	CHECK(Coordinated::outOfPlaneAngle(grandparent, parent, me, child) ==
	      doctest::Approx(30.f).epsilon(1e-3));
	CHECK(std::fabs(folded_torsion(grandparent, parent, me, child)) ==
	      doctest::Approx(30.f).epsilon(1e-3));
}

TEST_CASE("planar acceptance: elevation never exceeds the folded torsion, "
          "so it is only ever more permissive")
{
	glm::vec3 grandparent{-1.2f, 0.9f, 0.1f};
	glm::vec3 parent{0.f, 0.f, 0.f};
	glm::vec3 me{1.23f, 0.1f, -0.2f};

	int checked = 0;
	for (int i = 0; i < 2000; i++)
	{
		float s = (float)i;
		glm::vec3 child = me + glm::vec3{std::sin(s * 1.1f) * 2.f,
		                                 std::cos(s * 0.7f) * 2.f,
		                                 std::sin(s * 0.3f) * 2.f};

		float elevation = Coordinated::outOfPlaneAngle(grandparent, parent, me,
		                                               child);
		float torsion = std::fabs(folded_torsion(grandparent, parent, me,
		                                         child));

		CHECK(elevation >= 0.f);
		CHECK(elevation <= 90.001f);
		CHECK(elevation <= torsion + 0.01f);
		checked++;
	}

	CHECK(checked == 2000);
}

TEST_CASE("planar acceptance: a child in the plane has zero elevation, and "
          "collinear points do not divide by zero")
{
	glm::vec3 grandparent{-1.f, 1.f, 0.f};
	glm::vec3 parent{0.f, 0.f, 0.f};
	glm::vec3 me{1.5f, 0.f, 0.f};

	CHECK(Coordinated::outOfPlaneAngle(grandparent, parent, me,
	                                   me + glm::vec3{0.5f, -1.f, 0.f}) ==
	      doctest::Approx(0.f).epsilon(1e-3));

	// grandparent, parent and me on one line: no plane, so nothing to
	// object to
	float collinear = Coordinated::outOfPlaneAngle(
	glm::vec3{-1.f, 0.f, 0.f}, parent, me, me + glm::vec3{0.f, 1.f, 1.f});
	CHECK(std::isfinite(collinear));
	CHECK(collinear == doctest::Approx(0.f));
}
