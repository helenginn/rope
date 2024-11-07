#ifndef __vagabond__PathEntropy__
#define __vagabond__PathEntropy__

#include <stdlib.h>

struct Entropy {
int n_single;
int n_pair;
int n_nn;
double **h1;
double **sd1;
double **dm1;
double **hd1lm;
double **sd1lm;
double **dm1lm;
double *total;
double *sd_total;
double *dm_total;
};


struct Tors_res4nn {
	int n_ang;
};

struct Flag_par {
	int n;
	double minres;
	int kmi;
};


class PathEntropy
{
public:
	//PathEntropy();

	//~PathEntropy(){};

	/* Default flag parameters as chosen in pdb2entropy programme */
	void init_flag_par(struct Flag_par *flag_par);

	int alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct Flag_par flag_par);

	void get_atoms_and_residues(const std::string &model_id);

	// void calculate_entropy_independent(const std::string &model_id);
	
	int calculate_entropy_independent(int nf, struct Flag_par flag_par, struct Entropy *entropy);

	/* implicit compare function for qsort */
	static int comp (const void * elem1, const void * elem2);

	/* linear weighting function */
	int fitlw(double *x, double *y, double *w, int n, double *a, double *sd, int *ok);
};

#endif
