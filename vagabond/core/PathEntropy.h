#ifndef __vagabond__PathEntropy__
#define __vagabond__PathEntropy__

#include <stdlib.h>
#include <map>
#include <string>
#include <Sequence.h>
#include <PathGroup.h>

struct Entropy {
int n_single{};
int n_pair{};
int n_nn{};
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
double *total{};
double *sd_total{};
double *dm_total{};
double totalEntropy;
double sdTotalEntropy;
double dmeanTotal;
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

	struct Entropy* calculate_entropy_independent(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> tors_res);
    struct Entropy* calculate_entropy_mi(int nf, struct FlagParameters flagPar, std::vector<TorsRes4NN*> tors_res);

	/* implicit compare function for qsort */
	static int comp (const void * elem1, const void * elem2);

	/* linear weighting function */
	int fitlw(double *x, double *y, double *w, int n, double (&a)[3], double (&sd)[3], int *ok);

    void tors_res2mi(std::vector<TorsRes4NN*> tors_res, int resPerModel, std::vector<TorsRes4NN*> &tors_mi, int resPerModelMI, int *group2res, struct FlagParameters flagParameters);
	
    int alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct FlagParameters flagParameters);

    int allocVariables(int nf, double **ent_k, double **ent_k_tot, double **ent_k_2, double **ent_k_tot_2, double **sd_k, struct FlagParameters *flagParameters);
};

#endif
