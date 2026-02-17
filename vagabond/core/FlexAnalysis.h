#ifndef __vagabond__FlexAnalysis__
#define __vagabond__FlexAnalysis__

#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/TorsionData.h>
#include <stdlib.h>
#include <atomic>
#include <map>

#include "StructureModification.h"
#include "HBondManager.h"
#include "AtomGroup.h"

class Flexibility;
class Instance;

class FlexAnalysis {
public: 
	FlexAnalysis(Flexibility *flex, Instance *instance);
	// high-level workflow 
	void generateAtomCloud(int minCol, int maxCol, std::string flexTag);
	void calculateFreeEnergy();

	// utility / helpers
	void atomCloud(int minCol, int maxCol, float weight, const AtomVector &atoms);
	void savePositionsToCSV(const std::string &filename, const AtomVector &atoms) const;
	void saveBfactorsToCSV(const std::string &filename, const AtomVector &atoms) const;
	void saveFreeEnergyCSV(const std::string &filename,
	                            const std::vector<double> &enthalpies,
	                            const std::vector<double> &entropies,
	                            const std::vector<double> &freeEnergies) const;
	void calculateAnisoBfactors(const AtomVector &atoms);
	Eigen::Matrix3f covariance(const std::vector<glm::vec3> &samples) const;

	// Free-energy helpers
	double computeEnthalpy(double sigma) const;
	double computeEntropy(const std::vector<float>& v_i) const;


	// configuration 
	// void getColIdx()
	// {
	// 	int colIdx = _flex->_colIdx;
	// }

private:
    Flexibility* _flex;
    Instance *_instance = nullptr;
    std::string _flexTag;

};

#endif