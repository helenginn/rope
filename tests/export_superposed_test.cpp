#include <doctest/doctest.h>
#include <vagabond/core/Superpose.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/matrix_functions.h>
#include <fstream>
#include <map>
#include <unistd.h>

namespace
{
Atom *make_atom(int idx, const glm::vec3 &pos)
{
	Atom *atom = new Atom();
	atom->setAtomNum(idx + 1);
	atom->setAtomName("C" + std::to_string(idx + 1));
	atom->setCode("TST");
	atom->setElementSymbol("C");
	atom->setResidueId(std::to_string(idx + 1));
	atom->setChain("A");
	atom->setInitialPosition(pos, 20.f);
	return atom;
}

glm::mat4 snapshot_transform(float frac)
{
	glm::vec3 axis = glm::normalize(glm::vec3(0.4f, 0.7f, 0.2f));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.f), 0.6f * frac, axis);
	return glm::translate(glm::mat4(1.f),
	                      glm::vec3(3.f * frac, -2.f * frac, frac)) *
	       rotation;
}

void apply_snapshot(AtomGroup &group,
                    const std::map<Atom *, glm::vec3> &start,
                    float frac)
{
	glm::mat4 transform = snapshot_transform(frac);
	for (Atom *atom : group.atomVector())
	{
		glm::vec3 pos = glm::vec3(transform * glm::vec4(start.at(atom), 1.f));
		atom->setDerivedPosition(pos);
	}
}

bool near_vec(const glm::vec3 &a, const glm::vec3 &b, float tolerance = 1e-4f)
{
	for (size_t i = 0; i < 3; i++)
	{
		if (fabs(a[i] - b[i]) > tolerance)
		{
			return false;
		}
	}

	return true;
}
}

TEST_CASE("export superposed snapshots writes aligned atom positions")
{
	AtomGroup group;
	std::vector<glm::vec3> starts = {
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(1.2f, 0.1f, 0.4f),
		glm::vec3(-0.3f, 1.4f, 0.8f),
		glm::vec3(0.4f, -0.7f, 1.6f),
		glm::vec3(1.1f, 1.2f, -0.5f),
	};

	for (size_t i = 0; i < starts.size(); i++)
	{
		group.add(make_atom(i, starts[i]));
	}

	std::map<Atom *, glm::vec3> firstPos;
	for (Atom *atom : group.atomVector())
	{
		firstPos[atom] = atom->derivedPosition();
	}

	for (int i = 0; i < 3; i++)
	{
		float frac = i / 2.f;
		apply_snapshot(group, firstPos, frac);

		Superpose sp;
		for (Atom *atom : group.atomVector())
		{
			glm::vec3 start = firstPos[atom];
			glm::vec3 current = atom->derivedPosition();
			sp.addPositionPair(start, current);
		}

		sp.superpose();
		glm::mat4 tr = sp.transformation();

		std::map<Atom *, glm::vec3> originalPos;
		for (Atom *atom : group.atomVector())
		{
			glm::vec3 current = atom->derivedPosition();
			originalPos[atom] = current;
			glm::vec3 update = glm::vec3(tr * glm::vec4(current, 1.f));
			atom->setDerivedPosition(update);
			CHECK(near_vec(atom->derivedPosition(), firstPos[atom]));
		}

		std::string filename = "export_superposed_slice_" +
		                       std::to_string(i) + ".pdb";
		unlink(filename.c_str());

		group.writeToFile(filename);

		std::ifstream infile(filename);
		CHECK(infile.good());
		infile.close();
		CHECK(unlink(filename.c_str()) == 0);

		for (Atom *atom : group.atomVector())
		{
			atom->setDerivedPosition(originalPos[atom]);
			CHECK(near_vec(atom->derivedPosition(), originalPos[atom]));
		}
	}
}
