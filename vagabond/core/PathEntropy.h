#ifndef __vagabond__PathEntropy__
#define __vagabond__PathEntropy__

#include <stdlib.h>
#include <map>
#include <string>
#include <Sequence.h>
#include <PathGroup.h>

struct Entropy {
int nSingle{};
int nPairs{};
int nNearestNeighbours{};
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
    int mutualInformation;
	int kmi;
};


class PathEntropy
{
public:
	//PathEntropy();

	//~PathEntropy(){};

	/* default flag parameters as chosen in pdb2entropy programme */
	struct FlagParameters initFlagPar();

	std::vector<TorsRes4NN*> getAtomsAndResidues(const int numPaths, const std::vector<PathGroup> &paths);

	struct Entropy* calculateEntropyIndependent(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes);
    struct Entropy* calculateEntropyMI(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> torsRes);

	/* implicit compare function for qsort */
	static int comp (const void * elem1, const void * elem2);

	/* linear weighting function */
	int fitlw(double *x, double *y, double *w, int n, double (&a)[3], double (&sd)[3], int *ok);

    void torsRes2MI(std::vector<TorsRes4NN*> torsRes, int resPerModel, std::vector<TorsRes4NN*> &torsMI, int& resPerModelMI, int *group2res, struct FlagParameters flagParameters);
	
    int allocEntropy(struct Entropy *entropy, int nSingle, int nPairs, int nNearestNeighbours, struct FlagParameters flagParameters);

    int allocVariables(int nf, double **entk, double **entkTotal, double **entk2, double **entkTotal2, double **sigmak, struct FlagParameters *flagParameters);
};

#endif
