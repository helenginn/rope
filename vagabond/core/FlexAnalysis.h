#ifndef __vagabond__FlexAnalysis__
#define __vagabond__FlexAnalysis__

#include <algorithm>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/TorsionData.h>
#include <stdlib.h>
#include <atomic>
#include <map>
#include "StructureModification.h"
#include "HBondManager.h"
#include "Flexibility.h"


class FlexAnalysis {
public: 
	FlexAnalysis(Flexibility *flex);
	void generateAtomCloud();
	void atomCloud(float weight, const AtomVector &atoms);
	void savePositionsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms);
	void saveBfactorsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms);
	void calculateAnisoBfactors(std::string &_flexTag, const AtomVector &atoms);
	Eigen::Matrix3f covariance(const std::vector<glm::vec3> &samples);
	void calculateFreeEnergy();
	double computeEnthalpy(double sigma);
	double computeEntropy(const std::vector<float>& v_i);
	void saveFreeEnergyCSV(const std::string &filename,
                                    const std::vector<double> &enthalpies,
                                    const std::vector<double> &entropies,
                                    const std::vector<double> &freeEnergies);
	void getColIdx()
	{
		_colIdx = _flex->_colIdx;
	}
    void setFlexTag(std::string tagFlex)
    {
        std::string _flexTag = tagFlex;
    }
    voidsetColumnRange(int minCol, int maxCol)
	{
	    if (minCol > maxCol)
	    {
	        throw std::invalid_argument("minCol must be <= maxCol");
	    }
	    _minCol = minCol;
	    _maxCol = maxCol;
	}

private:
    Flexibility* _flex; 
    int _colIdx = 0;
    std::string _flexTag;
    const Eigen::MatrixXf& _V;
	const Eigen::MatrixXf& _S;


};