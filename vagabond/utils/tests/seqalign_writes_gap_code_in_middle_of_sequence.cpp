#include "../seqalign.h"

int main()
{
	std::string s("COWSEATSOMUCHGRASS");
	std::string t("COWSEATGRASSOFTEN");
	
	int muts = 0; int dels = 0;
	Alignment l, r;
	setup_alignment(&l, s);
	setup_alignment(&r, t);
	compare_sequences_and_alignments(s, t, &muts, &dels, l, r);
	
	int expected = 0;
	int wind = -1;
	wind_to_next_match(l, wind);
	wind_to_end_of_match(l, wind);
	int last = wind;

	std::ostringstream lss, ass, rss;
	IndexPairs indices;
	print_gap_between_alignments(l, r, lss, ass, rss, indices, wind);

	std::string code_gap = ass.str();
	
	print_map(l);
	print_map(r);
	
	std::cout << "Code gap: " << code_gap << std::endl;
	
	if (code_gap != "++++++")
	{
		return 1;
	}
	
	return 0;
}
