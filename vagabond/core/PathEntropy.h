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
double **h1lm{};
double **sd1lm{};
double **dm1lm{};
double *total{};
double *sd_total{};
double *dm_total{};
};


struct Tors_res4nn {
	int n_models{};
	int n_ang{};
	std::vector<std::string> *tors_name{};
	std::vector<std::vector<double>> **ang{};
	double *v{};
	int res_n{};
};

struct FlagParameters {
	int n{};
	double minres{};
	int kmi{};
};


class PathEntropy
{
public:
	//PathEntropy();

	//~PathEntropy(){};

	/* Default flag parameters as chosen in pdb2entropy programme */
	void init_flag_par();

	int alloc_tors(struct Tors_res4nn *tors_res, int seqSize);
	int alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct FlagParameters *flagParameters);

	Tors_res4nn* get_atoms_and_residues(const int numPaths, const std::vector<PathGroup> &paths, Sequence *seq);

	// void calculate_entropy_independent(const std::string &model_id);
	
	int calculate_entropy_independent(int nf, Sequence *seq, struct Tors_res4nn *tors_res, struct Entropy *entropy);

	/* implicit compare function for qsort */
	static int comp (const void * elem1, const void * elem2);

	/* linear weighting function */
	int fitlw(double *x, double *y, double *w, int n, double *a, double *sd, int *ok);
};

#endif
