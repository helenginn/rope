#ifndef __vagabond__FlexSample__
#define __vagabond__FlexSample__

#include <vector>
#include <string>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/StructureModification.h>
#include <vagabond/core/BondCalculator.h>

class Flexibility;
class Instance;

struct DeviationData {
    int sampleIdx;
    int modeIdx;
    float weight;
    int bondIdx;
    float startDist;
    float finalDist;
    float deviation;
};

class FlexSample : public StructureModification {
public: 
	FlexSample(Flexibility *flex, Instance *instance);
	void saveSampledStructures(int numSamples, const std::string& baseFileName, const std::string& csvDistFile);
	std::vector<int> sampleColumnIndices(int N, int sampleCount);
	std::vector<int> sampleColumnIndicesExp(int N, int sampleCount, double lambda);
	std::vector<int> getSoftestModeIndices(const Eigen::VectorXf& singularValues);
	void saveHierarchySamples(int numSamples, const std::string& baseFileName, float stepSize = 1.0f);
	void computeOneSample(int pickIdx, double weight);
    int pickIndex(const std::vector<int>& cumulativeWeights)
	{
	    float randomValue = static_cast<float>(rand()) / RAND_MAX;
	    float threshold = randomValue * cumulativeWeights.back();
	    return std::lower_bound(cumulativeWeights.begin(),
	                            cumulativeWeights.end(),
	                            threshold) - cumulativeWeights.begin();
	}
	// NEW: Helper function to write the CSV
    void saveBondDeviations(const std::vector<DeviationData>& data, const std::string& filename);
private:
    	Flexibility* _flex;
    	Instance *_instance = nullptr;

};

#endif