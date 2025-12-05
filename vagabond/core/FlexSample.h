#ifndef __vagabond__FlexSample__
#define __vagabond__FlexSample__

#include <algorithm>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/TorsionData.h>
#include <stdlib.h>
#include <atomic>
#include <map>
#include "StructureModification.h"
#include "HBondManager.h"
#include "Flexibility.h"


class FlexSample {
public: 
	FlexSample(Flexibility *flex);
	std::vector<int> sampleColumnIndices(int N, int sampleCount, double lambda);
	std::vector<int> getSoftestModeIndices(const Eigen::VectorXf& singularValues);
	void saveSampledStructures(int numSamples, const std::string& baseFileName, double lambda);
	std::vector<glm::vec3> makePosVec(const AtomVector &atoms);
	std::vector<float> makeRadiiVec(const AtomVector &atoms);
	std::set<std::pair<int,int>> makeExcList(OpSet<Atom*> &atom_set);
	std::set<std::pair<int,int>> makeExcHBonds(std::vector<Atom*> orderedAtoms, std::map<Atom*, int> indexing);
	bool checkClashes(const std::vector<Atom*> orderedAtoms, 
                                int saved,
                               const std::vector<float> &radii,
                               const std::set<std::pair<int,int>> &exclude,
                               float tolerance);
	void listClashes(const std::string &filename,
                              int saved,
                              const std::vector<Atom*> &orderedAtoms,
                              int i, int j,
                              const std::vector<float> &radii);

	private:
    	Flexibility* _flex;  
    	const Eigen::MatrixXf& _V;
		const Eigen::MatrixXf& _S;
};




};