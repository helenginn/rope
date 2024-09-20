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
} Entropy;


struct Flag_par {
	int n;
	double minres;
} Flag_par;

/* Default flag parameters as chosen in pdb2entropy programme */
void init_flag_par(struct Flag_par *flag_par)
{
	(*flag_par).n = 20; /* number of nearest neighbours */
	(*flag_par).minres = 1e-10;
}

void calculate_entropy_independent(const std::string &model_id);

/* implicit compare function for qsort */
int comp (const void * elem1, const void * elem2){
	double f1 = *((double *)elem1);
	double f2 = *((double *)elem2);
	if (f1 > f2) return 1;
	if (f1 < f2) return -1;
	return 0;
}

/* linear weighting function */
int fitlw(double *x, double *y, double *w, int n, double *a, double *sd, int *ok)
{
	int i, j, k;
	double xm, ym, x2, y2, xy, xy2, wt, sig2;
	wt = 0;
	xm = 0;
	ym = 0;
	x2 = 0;
	y2 = 0;
	xy = 0;

	for(i = 0; i < n; i++)
	{
		xm = xm + x[i]*w[i];
		ym = ym + y[i]*w[i];
		x2 = x2 + x[i]*x[i]*w[i];
		y2 = y2 + y[i]*y[i]*w[i];
		xy = xy + x[i]*y[i]*w[i];
		xy2 = xy2 + x[i]*y[i]*x[i]*y[i]*w[i];
		wt = wt + w[i];
	}

	xm = xm/wt;
	ym = ym/wt;
	x2 = x2/wt;
	y2 = y2/wt;
	xy = xy/wt;
	xm = xm/wt;

	a[1] = (xy - xm*ym)/(x2 - xm*xm);
	a[0] = ym - a[1]*xm;

	sig2 = 0;

	for(i = 0; i < n; i++)
	{
		sig2 = sig2 + (y[i] - a[0] - a[1]*x[i])*(y[i] - a[0] - a[1]*x[i])*w[i];
	}

	sig2 = sig2/wt;

	sd[0] = sqrt(sig2 * (double) n / (double) (n-2)) * sqrt((1.0/(double) n) + xm*xm/((double) n * (x2 - xm*xm)));
	sd[1] = sqrt(sig2 * (double) n / (double) (n-2)) / sqrt((double) n * (x2 - xm*xm));
}
