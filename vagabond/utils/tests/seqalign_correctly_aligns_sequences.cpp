#include "../seqalign.h"

int main()
{
	std::string s("ALLCOWSEATGRASSOCCASIONALLY");
	std::string t("ITHINKALLCOWSMAYEATGRASS");
	
	int muts = 0; int dels = 0;
	Alignment l, r;
	setup_alignment(&l, s);
	setup_alignment(&r, t);
	compare_sequences_and_alignments(s, t, &muts, &dels, l, r);
	
	print_map(l);
	print_map(r);
	
	std::cout << s << std::endl;
	std::cout << t << std::endl;
	std::cout << std::endl;

	std::ostringstream lss, ass, rss;
	IndexPairs indices;
	print_alignments(l, r, lss, ass, rss, indices);

	delete_alignment(&l);
	delete_alignment(&r);
	
	std::cout << lss.str() << std::endl;
	std::cout << ass.str() << std::endl;
	std::cout << rss.str() << std::endl;
	
	if (lss.str() != "      ALLCOWS   EATGRASSOCCASIONALLY")
	{
		return 1;
	}
	
	if (ass.str() != "------.......---........++++++++++++")
	{
		return 1;
	}
	
	if (rss.str() != "ITHINKALLCOWSMAYEATGRASS            ")
	{
		return 1;
	}
	
	return 0;
}
