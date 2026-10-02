#include <doctest/doctest.h>
#include <vagabond/core/protonic/hnet.h>
#include <vagabond/utils/OpSet.h>
#include <bit>
#include <random>
#include <vector>

using namespace hnet;

// The allocation-free count helpers used by CountAdder, BondAdder and
// BreakMatrix must give exactly what the vector/OpSet versions they replaced
// gave - these compare them on many bit patterns, including the awkward ones
// (nothing, everything, bit 31, values past +-15).

namespace
{

std::vector<unsigned int> sample_masks()
{
	std::vector<unsigned int> masks = {0u, ~0u, 1u, 1u << 15, 1u << 16,
	                                   1u << 30, 1u << 31, 0xFFFFu,
	                                   0xFFFF0000u, 0x80000001u};
	std::mt19937 rng(12345);
	for (int i = 0; i < 3000; i++)
	{
		unsigned int m = rng();
		// sparse ones too: most real beliefs have only a few options left
		masks.push_back(i % 2 ? m : (m & rng() & rng()));
	}
	return masks;
}

}

TEST_CASE("possible_values_list matches possible_values")
{
	for (unsigned int mask : sample_masks())
	{
		Count::Values count = (Count::Values)mask;
		std::vector<int> expected = possible_values(count);
		CountList got = possible_values_list(count);

		REQUIRE(got.size() == expected.size());
		for (size_t i = 0; i < expected.size(); i++)
		{
			REQUIRE(got[i] == expected[i]);
		}
	}
}

TEST_CASE("combine_as_count matches values_as_count over every pair")
{
	std::vector<unsigned int> masks = sample_masks();
	std::mt19937 rng(777);

	for (int n = 0; n < 4000; n++)
	{
		Count::Values a = (Count::Values)masks[rng() % masks.size()];
		Count::Values b = (Count::Values)masks[rng() % masks.size()];

		std::vector<int> as = possible_values(a);
		std::vector<int> bs = possible_values(b);

		std::vector<int> sums, diffs;
		for (int x : as)
		{
			for (int y : bs)
			{
				sums.push_back(x + y);
				diffs.push_back(x - y);
			}
		}

		CountList la = possible_values_list(a);
		CountList lb = possible_values_list(b);

		REQUIRE(combine_as_count(la, lb, [](const int &x, const int &y)
		                         { return x + y; }) == values_as_count(sums));
		REQUIRE(combine_as_count(la, lb, [](const int &x, const int &y)
		                         { return x - y; }) == values_as_count(diffs));
	}
}

// BondAdder::check()'s "which totals could the sum take" step, written the
// old way (ordered sets) against the bitmask way now in BondAdder.h.
TEST_CASE("BondAdder totals as bitmasks match the set version")
{
	std::vector<unsigned int> masks = sample_masks();

	for (int certain = 0; certain <= 20; certain++)
	{
		for (int maybe = 0; maybe <= 20; maybe++)
		{
			for (size_t m = 0; m < masks.size(); m += 7)
			{
				Count::Values sum = (Count::Values)masks[m];

				OpSet<int> possibilities;
				for (int i = certain; i <= certain + maybe; i++)
				{
					possibilities.insert(i);
				}

				Count::Values oldCount = values_as_count(possibilities);
				OpSet<int> sum_options;
				for (int v : possible_values(sum)) { sum_options.insert(v); }
				OpSet<int> common = possibilities.common_to_both(sum_options);

				unsigned int bits = 0;
				for (int i = certain; i <= certain + maybe && i <= 15; i++)
				{
					bits |= (1u << i);
				}
				unsigned int newCommon = bits & (unsigned int)sum & 0xFFFFu;

				REQUIRE((Count::Values)bits == oldCount);
				REQUIRE((size_t)std::popcount(newCommon) == common.size());
				if (common.size() == 1)
				{
					REQUIRE(std::countr_zero(newCommon) == *common.begin());
				}
			}
		}
	}
}

TEST_CASE("certain() is true for at most one set bit")
{
	for (unsigned int mask : sample_masks())
	{
		bool expected = std::popcount(mask) <= 1;
		REQUIRE(certain((Count::Values)mask) == expected);
	}
}
