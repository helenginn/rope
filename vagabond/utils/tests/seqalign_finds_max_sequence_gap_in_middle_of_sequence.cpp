#include "../seqalign.h"

int main()
{
	std::string s("COWSEATSOMUCHGRASS");
	std::string t("ALLCOWSEATGRASS");
	
	int muts = 0; int dels = 0;
	Alignment l, r;
	setup_alignment(&l, s);
	setup_alignment(&r, t);
	compare_sequences_and_alignments(s, t, &muts, &dels, r, l);
	
	int expected = 6;
	int wind = -1;
	wind_to_next_match(l, wind);
	wind_to_end_of_match(l, wind);
	int last = wind;
	wind++;
	wind_to_next_match(l, wind);

	std::cout << "Last: " << last << std::endl;
	std::cout << "Wind: " << wind << std::endl;
	int gap = max_sequence_gap(l, r, last, wind);
	
	print_map(l);
	print_map(r);
	
	std::cout << "Expected: " << expected << std::endl;
	std::cout << "Max sequence gap: " << gap << std::endl;
	
	return !(gap == expected);
}
