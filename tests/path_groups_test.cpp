#include <doctest/doctest.h>
#include <vagabond/core/PathManager.h>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

static json fake_path(const std::string &start, const std::string &end,
                      const std::string &hash)
{
	json j;
	j["start"] = start;
	j["end"] = end;
	j["model"] = "no_such_model";
	j["motions"] = json::array();
	j["hash"] = hash;
	j["clash_score"] = 1.f;
	j["momentum_score"] = 2.f;
	return j;
}

static void add_path(PathManager &pm, const std::string &start,
                     const std::string &end, const std::string &hash)
{
	Path p = fake_path(start, end, hash);
	pm.objects().push_back(p);
}

static std::set<std::string> files_in(const fs::path &dir)
{
	std::set<std::string> names;
	for (const auto &entry : fs::directory_iterator(dir))
	{
		names.insert(entry.path().filename().string());
	}
	return names;
}

struct TempDir
{
	fs::path path;
	TempDir()
	{
		path = fs::temp_directory_path() / "rope_path_groups_test";
		fs::remove_all(path);
	}
	~TempDir() { fs::remove_all(path); }
};

TEST_CASE("path groups: save, reload and incremental rewrite")
{
	TempDir tmp;
	std::string dir = tmp.path.string();

	PathManager pm;
	add_path(pm, "a", "b", "h1");
	add_path(pm, "a", "b", "h2");
	add_path(pm, "a", "c/odd name", "h3");

	pm.savePathGroups(dir);
	std::set<std::string> files = files_in(tmp.path);
	CHECK(files.size() == 2);

	for (const Path &p : pm.objects())
	{
		CHECK(!p.isDirty());
	}

	SUBCASE("reload gives the same paths, all clean")
	{
		PathManager other;
		CHECK(other.loadPathGroups(dir) == 3);
		CHECK(other.objectCount() == 3);

		std::multiset<std::string> ends;
		for (const Path &p : other.objects())
		{
			ends.insert(p.endId());
			CHECK(!p.isDirty());
		}
		CHECK(ends.count("b") == 2);
		CHECK(ends.count("c/odd name") == 1);
	}

	SUBCASE("unchanged groups are not rewritten")
	{
		for (const std::string &f : files)
		{
			fs::remove(tmp.path / f);
		}

		pm.savePathGroups(dir);
		CHECK(files_in(tmp.path).empty());

		// touching one path rewrites only its group
		pm.objects().back().setDirty(true);
		pm.savePathGroups(dir);
		CHECK(files_in(tmp.path).size() == 1);
	}

	SUBCASE("a group which loses a path is rewritten")
	{
		pm.objects().pop_front();
		pm.savePathGroups(dir);

		PathManager other;
		CHECK(other.loadPathGroups(dir) == 2);
	}

	SUBCASE("a group with no paths left loses its file")
	{
		pm.objects().pop_back();
		pm.savePathGroups(dir);
		CHECK(files_in(tmp.path).size() == 1);

		PathManager other;
		CHECK(other.loadPathGroups(dir) == 2);
	}
}

TEST_CASE("path groups: legacy single-file paths stay dirty")
{
	json legacy;
	legacy["paths"] = json::array({fake_path("a", "b", "h1")});

	PathManager pm = legacy;
	REQUIRE(pm.objectCount() == 1);
	CHECK(pm.objects().front().isDirty());

	// no paths array means paths come from the directory instead
	json modern = pm;
	CHECK(!modern.count("paths"));
}
