#include <doctest/doctest.h>

#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomContent.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/ModelManager.h>
#include <vagabond/core/NonCovalents.h>
#include <vagabond/core/paths/NewPath.h>
#include <vagabond/core/paths/PlausibleRoute.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace
{
constexpr float endpoint_rmsd_tolerance = 0.02f;
constexpr float endpoint_max_tolerance = 0.08f;

std::filesystem::path capsid_project_dir()
{
	const char *env = std::getenv("ROPE_CAPSID_PROJECT");
	if (env && env[0])
	{
		return env;
	}

	return "/Users/admin/rope/capsid/rope";
}

std::string stripped(std::string str)
{
	while (!str.empty() && std::isspace((unsigned char)str.front()))
	{
		str.erase(str.begin());
	}
	while (!str.empty() && std::isspace((unsigned char)str.back()))
	{
		str.pop_back();
	}

	return str;
}

std::string atom_key(Atom *atom)
{
	return atom->chain() + "|" + atom->residueId().str() + "|" +
	       atom->code() + "|" + atom->atomName();
}

struct AtomPositions
{
	std::map<std::string, glm::vec3> positions;
};

AtomPositions positions_from_atom_group(AtomGroup *atoms, bool derived)
{
	AtomPositions ret;

	for (Atom *atom : atoms->atomVector())
	{
		ret.positions[atom_key(atom)] = derived ? atom->derivedPosition() :
		                                atom->initialPosition();
	}

	return ret;
}

AtomPositions positions_from_pdb(const std::filesystem::path &path)
{
	struct PdbAtom
	{
		std::string chain;
		std::string residue;
		std::string code;
		std::string name;
		int model = 0;
		glm::vec3 position{};
	};

	std::vector<PdbAtom> atoms;
	std::ifstream file(path);
	REQUIRE_MESSAGE(file.good(), "Could not open PDB file " << path);

	std::string line;
	int model = -1;
	int model_count = 0;
	while (std::getline(file, line))
	{
		if (line.rfind("MODEL", 0) == 0)
		{
			model++;
			model_count = std::max(model_count, model + 1);
			continue;
		}

		if (line.rfind("ATOM  ", 0) != 0 && line.rfind("HETATM", 0) != 0)
		{
			continue;
		}

		if (line.size() < 54)
		{
			continue;
		}

		const int atom_model = model >= 0 ? model : 0;
		model_count = std::max(model_count, atom_model + 1);

		PdbAtom atom;
		atom.name = stripped(line.substr(12, 4));
		atom.code = stripped(line.substr(17, 3));
		atom.chain = stripped(line.substr(21, 1));
		atom.residue = stripped(line.substr(22, 4));
		if (line.size() > 26 && !std::isspace((unsigned char)line[26]))
		{
			atom.residue += line[26];
		}
		atom.model = atom_model;
		atom.position.x = std::stof(line.substr(30, 8));
		atom.position.y = std::stof(line.substr(38, 8));
		atom.position.z = std::stof(line.substr(46, 8));
		atoms.push_back(atom);
	}

	AtomPositions ret;
	for (const PdbAtom &atom : atoms)
	{
		std::string chain = atom.chain;
		if (model_count > 1)
		{
			chain += std::to_string(atom.model);
		}

		const std::string key = chain + "|" + atom.residue + "|" +
		                        atom.code + "|" + atom.name;
		ret.positions[key] = atom.position;
	}

	return ret;
}

struct Comparison
{
	size_t matched = 0;
	size_t missing = 0;
	size_t extra = 0;
	float rmsd = 0;
	float max_distance = 0;
	std::string worst_atom;
	std::vector<std::string> missing_examples;
	std::vector<std::string> extra_examples;
};

Comparison compare_positions(const AtomPositions &actual,
                             const AtomPositions &expected)
{
	Comparison ret;
	double sum_sq = 0;

	for (const auto &[key, expected_pos] : expected.positions)
	{
		auto it = actual.positions.find(key);
		if (it == actual.positions.end())
		{
			ret.missing++;
			if (ret.missing_examples.size() < 5)
			{
				ret.missing_examples.push_back(key);
			}
			continue;
		}

		const glm::vec3 delta = it->second - expected_pos;
		const float distance = glm::length(delta);
		sum_sq += distance * distance;
		ret.matched++;

		if (distance > ret.max_distance)
		{
			ret.max_distance = distance;
			ret.worst_atom = key;
		}
	}

	for (const auto &[key, actual_pos] : actual.positions)
	{
		if (!expected.positions.count(key))
		{
			ret.extra++;
			if (ret.extra_examples.size() < 5)
			{
				ret.extra_examples.push_back(key);
			}
		}
	}

	if (ret.matched)
	{
		ret.rmsd = std::sqrt(sum_sq / (double)ret.matched);
	}

	return ret;
}

void check_comparison(const Comparison &comparison)
{
	INFO("matched atoms: " << comparison.matched);
	INFO("missing atoms: " << comparison.missing);
	INFO("extra atoms: " << comparison.extra);
	INFO("RMSD: " << comparison.rmsd);
	INFO("max distance: " << comparison.max_distance);
	INFO("worst atom: " << comparison.worst_atom);

	for (const std::string &example : comparison.missing_examples)
	{
		INFO("missing example: " << example);
	}

	for (const std::string &example : comparison.extra_examples)
	{
		INFO("extra example: " << example);
	}

	CHECK(comparison.missing == 0);
	CHECK(comparison.extra == 0);
	CHECK(comparison.rmsd < endpoint_rmsd_tolerance);
	CHECK(comparison.max_distance < endpoint_max_tolerance);
}

void check_exported_endpoint(const std::filesystem::path &project,
                             const std::string &actual_name,
                             const std::string &expected_name)
{
	const std::filesystem::path actual_path = project / actual_name;
	const std::filesystem::path expected_path = project / expected_name;

	if (!std::filesystem::exists(actual_path))
	{
		MESSAGE("Skipping missing exported endpoint: " << actual_path);
		return;
	}
	if (!std::filesystem::exists(expected_path))
	{
		MESSAGE("Skipping missing target endpoint: " << expected_path);
		return;
	}

	INFO("actual: " << actual_path);
	INFO("expected: " << expected_path);
	check_comparison(compare_positions(positions_from_pdb(actual_path),
	                                   positions_from_pdb(expected_path)));
}

std::string mapped_end_id(const std::string &start_id)
{
	const std::string start = "contracted_all_matched";
	const std::string end = "expanded_all_matched";

	if (start_id.rfind(start, 0) != 0)
	{
		return "";
	}

	return end + start_id.substr(start.size());
}

struct ScopedCurrentPath
{
	ScopedCurrentPath(const std::filesystem::path &next)
	{
		old = std::filesystem::current_path();
		std::filesystem::current_path(next);
	}

	~ScopedCurrentPath()
	{
		std::filesystem::current_path(old);
	}

	std::filesystem::path old;
};
}

TEST_CASE("capsid exported path endpoint agrees with expanded model")
{
	const std::filesystem::path project = capsid_project_dir();
	if (!std::filesystem::exists(project))
	{
		MESSAGE("Skipping missing capsid project: " << project);
		return;
	}

	check_exported_endpoint(project,
	                        "contracted_all_matched_2_expanded_all_matched_slice_1.pdb",
	                        "expanded_all_matched.pdb");

	check_exported_endpoint(project,
	                        "expanded_all_matched_2_contracted_all_matched_slice_1.pdb",
	                        "contracted_all_matched.pdb");
}

TEST_CASE("capsid pathfinding endpoint agrees with expanded model")
{
	const char *run = std::getenv("ROPE_RUN_CAPSID_PATHFINDING_TEST");
	if (!run || !run[0])
	{
		MESSAGE("Skipping full capsid pathfinding endpoint check; set "
		        "ROPE_RUN_CAPSID_PATHFINDING_TEST=1 to run it.");
		return;
	}

	const std::filesystem::path project = capsid_project_dir();
	if (!std::filesystem::exists(project / "rope.json"))
	{
		MESSAGE("Skipping missing capsid rope.json: " << (project / "rope.json"));
		return;
	}

	ScopedCurrentPath cwd(project);
	Environment::env().load("rope.json");

	Model *from = Environment::modelManager()->model("contracted_all_matched");
	Model *to = Environment::modelManager()->model("expanded_all_matched");
	REQUIRE(from != nullptr);
	REQUIRE(to != nullptr);

	NewPath new_path;
	size_t mapped = 0;
	for (Instance *start : from->instances())
	{
		Instance *end = to->instanceWithId(mapped_end_id(start->id()));
		REQUIRE_MESSAGE(end != nullptr, "No end instance for " << start->id());
		new_path.addLinkedInstances(start, end);
		mapped++;
	}
	REQUIRE(mapped > 0);

	std::unique_ptr<NonCovalents> noncovs(new NonCovalents());
	std::unique_ptr<PlausibleRoute> route(new_path());
	route->setNonCovalents(noncovs.get());
	route->setMaximumFlipTrial(0);
	route->setThreads(1);
	route->setup();

	to->load(Model::NoGeometry);
	AtomPositions expected = positions_from_atom_group(to->currentAtoms(), false);

	route->submitJobAndRetrieve(1.0f, true);
	AtomPositions actual = positions_from_atom_group(from->currentAtoms(), true);

	check_comparison(compare_positions(actual, expected));
}
