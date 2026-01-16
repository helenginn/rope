#ifndef __vagabond__PathEntropy__
#define __vagabond__PathEntropy__

#include <stdlib.h>
#include <map>
#include <string>
#include <Path.h>

struct Entropy {
int nSingle{};
int nPairs{};
int nNearestNeighbours{};
int nEdges{};
double **h1{};
double **sd1{};
double **dm1{};
double *h1lm{};
double *sd1lm{};
double *dm1lm{};
int *i1{};
int *i2{};
double **h2{};
double **sd2{};
double **dm2{};
double *h2lm{};
double *sd2lm{};
double *dm2lm{};
double **mi{};
double *milm{};
double **dmmi{};
double **sdmi{};
int *mst1{};
int *mst2{};
double *mstw{};
double *pathTotal{};
double *sigmaTotal{};
double *meanDistTotal{};
double totalEntropy;
double sigmaTotalEntropy;
double meanDistTotalEntropy;
double *entResidue{};
};


struct TorsRes4NN {
	int nModels{};
	int nAng{};
    std::vector<double> bondSymmetry{};
	std::vector<std::string> torsName{};
    std::vector<std::string> desc{};
    std::vector<std::vector<double>> ang{};
	std::vector<glm::vec3> v{};
	int resID{};
};

struct FlagParameters {
	int n;
    int ne;
	double minres;
    float cutoff;
    bool mist;
    int mutualInformation;
	int kmi;
};

struct Edge {
    double weight;
    int u;
    int v;
    int orig;
};

class PathEntropy
{
public:
	PathEntropy(){};
    ~PathEntropy(){};

	/* default flag parameters as chosen in pdb2entropy programme */
	struct FlagParameters initFlagPar();

	std::vector<TorsRes4NN*> getAtomsAndResidues(const int numPaths, const std::vector<Path*> paths);

	struct Entropy* calculateEntropyIndependent(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes);
    struct Entropy* calculateEntropyMI(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes);

	/* linear weighting function */
	int fitlw(double *x, double *y, double *w, int n, double (&a)[3], double (&sd)[3], int *ok);

    int torsRes2MI(std::vector<TorsRes4NN*> torsRes, int resPerModel, std::vector<TorsRes4NN*> &torsMI, int& resPerModelMI, int *group2res, struct FlagParameters flagParameters);

    /* kruskal algorithm */
    void kruskal(struct Entropy *entropy, int *group2res, struct FlagParameters flagParameters);
	
    int allocEntropy(struct Entropy *entropy, int nSingle, int nPairs, int nNearestNeighbours, struct FlagParameters flagParameters);

    int allocVariables(int nf, double **entk, double **entkTotal, double **entk2, double **entkTotal2, double **sigmak, struct FlagParameters *flagParameters);
};

#endif
