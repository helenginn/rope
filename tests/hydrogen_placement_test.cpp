#include <doctest/doctest.h>
#include "glm_printing.h"
#include <vagabond/core/protonic/Probe.h>

namespace
{

// a hydrogen probe on a bare atom at (1, 0, 0) - its construction position,
// standing in for the midpoint of the two heavy atoms
struct Fixture
{
	Atom hydrogen{"HOH", "H1"};
	Atom donorAtom{"HOH", "O"};
	Atom otherAtom{"ASP", "N"};

	hnet::ExistenceConnector obj;
	hnet::ExistenceConnector exist;

	Fixture()
	{
		hydrogen.setInitialPosition(glm::vec3(1.f, 0.f, 0.f));

		// HydrogenProbe reads the first conformer's letter
		hydrogen.conformerPositions()[""];

		// setAtomPosition() tells the GUI through this, unguarded
		obj.set_update([](bool) {});
	}

	hnet::AtomConf donor()
	{
		return {&donorAtom, '\0'};
	}

	hnet::AtomConf other()
	{
		return {&otherAtom, '\0'};
	}
};

}

TEST_CASE("hydrogen placement: a new hydrogen is unplaced, at its midpoint")
{
	Fixture f;
	HydrogenProbe probe(f.obj, f.exist, &f.hydrogen);

	CHECK(!probe._placed);
	CHECK(!probe.placedOn(f.donor()));
	CHECK(probe.position() == glm::vec3(1.f, 0.f, 0.f));
}

TEST_CASE("hydrogen placement: placing records where and by whom")
{
	Fixture f;
	HydrogenProbe probe(f.obj, f.exist, &f.hydrogen);

	GuiltVersion before = probe._lastPositioned;
	probe.place(glm::vec3(0.2f, 0.9f, 0.f), f.donor());

	CHECK(probe._placed);
	CHECK(probe.position() == glm::vec3(0.2f, 0.9f, 0.f));
	CHECK(f.hydrogen.initialPosition() == glm::vec3(0.2f, 0.9f, 0.f));

	// on the atom that placed it, and only on that one
	CHECK(probe.placedOn(f.donor()));
	CHECK(!probe.placedOn(f.other()));

	// and the position stamp moved, so anything cached on it is recomputed
	CHECK(probe._lastPositioned != before);
}

TEST_CASE("hydrogen placement: unplacing puts it back at the midpoint")
{
	Fixture f;
	HydrogenProbe probe(f.obj, f.exist, &f.hydrogen);

	probe.place(glm::vec3(0.2f, 0.9f, 0.f), f.donor());
	GuiltVersion placed_at = probe._lastPositioned;

	probe.unplace();

	CHECK(!probe._placed);
	CHECK(!probe.placedOn(f.donor()));
	CHECK(probe.position() == glm::vec3(1.f, 0.f, 0.f));
	CHECK(f.hydrogen.initialPosition() == glm::vec3(1.f, 0.f, 0.f));
	CHECK(probe._lastPositioned != placed_at);
}

TEST_CASE("hydrogen placement: unplacing an unplaced hydrogen changes nothing")
{
	Fixture f;
	HydrogenProbe probe(f.obj, f.exist, &f.hydrogen);

	GuiltVersion before = probe._lastPositioned;
	probe.unplace();
	probe.unplace();

	CHECK(!probe._placed);
	CHECK(probe._lastPositioned == before); // no churn
}

TEST_CASE("hydrogen placement: a hydrogen can move from one donor to the "
          "other and back")
{
	Fixture f;
	HydrogenProbe probe(f.obj, f.exist, &f.hydrogen);

	probe.place(glm::vec3(0.1f, 0.9f, 0.f), f.donor());
	CHECK(probe.placedOn(f.donor()));

	// the other atom becomes the donor and places it on its own side
	probe.place(glm::vec3(1.9f, 0.9f, 0.f), f.other());
	CHECK(probe.placedOn(f.other()));
	CHECK(!probe.placedOn(f.donor()));

	probe.unplace();
	CHECK(!probe.placedOn(f.other()));
	CHECK(!probe.placedOn(f.donor()));
}
