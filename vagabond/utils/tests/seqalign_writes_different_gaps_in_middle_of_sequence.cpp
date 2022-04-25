#include "../seqalign.h"

int main()
{
	std::string s("COWSEATSOMUCHGRASS");
	std::string t("COWSEATWONDERFULGRASS");
	
	int muts = 0; int dels = 0;
	Alignment l, r;
	setup_alignment(&l, s);
	setup_alignment(&r, t);
	compare_sequences_and_alignments(s, t, &muts, &dels, l, r);
	
	print_map(l);
	print_map(r);

	
	int expected = 0;
	int wind = -1;
	wind_to_next_match(l, wind);
	wind_to_end_of_match(l, wind);
	int last = wind;

	std::ostringstream lss, ass, rss;
	IndexPairs indices;
	print_gap_between_alignments(l, r, lss, ass, rss, indices, wind);

	std::string left_gap = lss.str();
	std::string code_gap = ass.str();
	std::string right_gap = rss.str();
	
	std::cout << "Left gap:  " << left_gap << std::endl;
	std::cout << "Code gap:  " << code_gap << std::endl;
	std::cout << "Right gap: " << right_gap << std::endl;
	
	if (code_gap != "      ---")
	{
		return 1;
	}
	
	return 0;
}
