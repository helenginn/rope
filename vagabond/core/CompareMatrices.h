#ifndef __vagabond__CompareMatrices__
#define __vagabond__CompareMatrices__

#include <cmath>
#include <string>
#include <utility>
#include <vector>
#include "MatrixCache.h"


struct ComparisonStats
{
	struct Pair
	{
		ResidueId i, j;
		double value1, value2; 
		double z1 = 0, z2 = 0; // standardised values
	};

	std::string label1, label2; 
	std::vector<ResidueId> common; // residues in both matrices, in order
	std::vector<std::pair<ResidueId, std::string>> excluded; 
	std::vector <Pair> pairs; 
	double pearson = NAN;
	double spearman = NAN; 
	// pair agreement z1*z2, common.size()common.size()
	std::vector<double> agreement;
};

class CompareMatrices
{
public:
	CompareMatrices(const SlotMatrix &m1, const SlotMatrix &m2);
	void run();
	const ComparisonStats &stats() const
	{
		return _stats;
	}
	bool writeCSVs(const std::string &prefix) const;
private:
	void matchResidues();
	void collectPairs();
	void calculateScores();
	void standardise();
	void buildAgreementMap();

	SlotMatrix _m1;
	SlotMatrix _m2;
	ComparisonStats _stats;

	std::vector<int> _rows1;
	std::vector<int> _rows2;

};



#endif 