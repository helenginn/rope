#include <doctest/doctest.h>
#include <vagabond/core/PathManager.h>
#include <set>

TEST_CASE("path pairs: auto-paths visits the whole square, not a triangle")
{
	const size_t n = 4;
	std::vector<std::pair<int, int>> pairs = PathManager::allPairs(n);

	CHECK(pairs.size() == n * n);

	std::set<std::pair<int, int>> unique(pairs.begin(), pairs.end());
	CHECK(unique.size() == n * n);

	for (int i = 0; i < (int)n; i++)
	{
		for (int j = 0; j < (int)n; j++)
		{
			// both directions, the diagonal, and the first column
			CHECK(unique.count({i, j}) == 1);
		}
	}
}

TEST_CASE("path pairs: ordered by first instance, then second")
{
	std::vector<std::pair<int, int>> pairs = PathManager::allPairs(3);

	REQUIRE(pairs.size() == 9);
	CHECK(pairs[0] == std::make_pair(0, 0));
	CHECK(pairs[1] == std::make_pair(0, 1));
	CHECK(pairs[2] == std::make_pair(0, 2));
	CHECK(pairs[3] == std::make_pair(1, 0));
	CHECK(pairs[8] == std::make_pair(2, 2));
}

TEST_CASE("path pairs: nothing to pair gives nothing")
{
	CHECK(PathManager::allPairs(0).size() == 0);
	CHECK(PathManager::allPairs(1).size() == 1);
}
