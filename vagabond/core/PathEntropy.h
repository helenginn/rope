#ifndef __vagabond__PathEntropy__
#define __vagabond__PathEntropy__

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include <Path.h>

//struct for calculation using the k-NN method with Kozachenko-Leonenko estimator
struct EntropyForMatrix {
    std::vector<double> totalEntropy{};
};

struct TorsRes4NN {
	int nModels{};
	int nAng{};
    std::vector<double> bondSymmetry{};
	std::vector<std::string> torsName{};
    std::vector<std::string> desc{};
    std::vector<std::vector<std::vector<double>>> ang{};
	std::vector<glm::vec3> v{};
	int resID{};
    std::string resName{};
};

struct FlagParameters {
	int n; //number of nearest neighbours
    int nf; //number of paths considered
    int ne;
	double minres;
    float cutoff;
    int timeDivisions;
    bool mist;
	int kmi;
};

struct Edge {
    double weight;
    int u;
    int v;
    int orig;
};

struct EntropyResults
{
    double totalEntropy{};
    double sigmaTotalEntropy{};
    double meanDistTotalEntropy{};
    std::vector<double> pathTotal{};
    std::vector<double> sigmaTotal{};
    std::vector<double> meanDistTotal{};
    std::vector<double> entResidue{};
};

class PathEntropy
{
public:
	PathEntropy(){};
    ~PathEntropy(){};

	/* default flag parameters as chosen in pdb2entropy programme */
	struct FlagParameters initFlagPar();

	std::vector<TorsRes4NN*> getAtomsAndResidues(const int numPaths, const std::vector<Path*> paths, int numDivisions = 1);

	void calculateEntropyIndependent(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes, int numDivisions = 1);
    void calculateEntropyMI(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes, int numDivisions = 1);

    const std::vector<EntropyResults>& result() const
    {
        return _results;
    }

private:

	struct EntropyKL
    {
		int nSingle{};
		int nPairs{};
		int nEdges{};
		int nNearestNeighbours{};
		
    	std::vector<std::vector<double>> h1{};
	    std::vector<std::vector<double>> sd1{};
		std::vector<std::vector<double>> dm1{};
		std::vector<double> h1lm{};
		std::vector<double> sd1lm{};
		std::vector<double> dm1lm{};
		
        std::vector<int> i1{};
		std::vector<int> i2{};
		std::vector<std::vector<double>> h2{};
		std::vector<std::vector<double>> sd2{};
		std::vector<std::vector<double>> dm2{};
		std::vector<double> h2lm{};
		std::vector<double> sd2lm{};
		std::vector<double> dm2lm{};
		std::vector<double> milm{};
		std::vector<std::vector<double>> mi{};
		std::vector<std::vector<double>> dmmi{};
		std::vector<std::vector<double>> sdmi{};
		
        std::vector<int> mst1{};
		std::vector<int> mst2{};
		std::vector<double> mstw{};
		
        std::vector<double> pathTotal{};
		std::vector<double> sigmaTotal{};
		std::vector<double> meanDistTotal{};
		
        double totalEntropy{};
		double sigmaTotalEntropy{};
		double meanDistTotalEntropy{};
		
        std::vector<double> entResidue{};
	};

	/* linear weighting function */
	void fitlw(std::vector<double> x, std::vector<double> y, std::vector<double> w, int n, std::vector<double> &a, std::vector<double> &sd);

    void torsRes2MI(std::vector<TorsRes4NN*> torsRes, int resPerModel, std::vector<TorsRes4NN*> &torsMI, int& resPerModelMI, std::vector<int>& group2res, struct FlagParameters flagParameters, int timeDivisions = 1);

    void kNearestNeighbours(std::vector<TorsRes4NN*> torsRes, EntropyKL& entropy, struct FlagParameters flagParameters, int &nTors, int nf, int numResPerModel, int K, int timeDivisions = 1);

    /* kruskal algorithm */
    void kruskal(struct EntropyKL& entropy, std::vector<int>& group2res, struct FlagParameters flagParameters);
	
    void allocEntropy(struct EntropyKL& entropy, int nSingle, int nPairs, int nNearestNeighbours, struct FlagParameters flagParameters);

    void allocVariables(int nf, std::vector<double> &entk, std::vector<double> &entkTotal, std::vector<double> &entk2, std::vector<double> &entkTotal2, std::vector<double> &sigmak, struct FlagParameters& flagParameters);

    EntropyResults getResults(const EntropyKL& entropy) const;

    std::vector<EntropyResults> _results;
};

#endif
