#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <numeric>
#include <math.h>
#include <../utils/degrad.h>
#include <matrix_functions.h>
#include <PathEntropy.h>
#include <Sequence.h>
#include <TorsionRef.h>
#include <Residue.h>
#include <Environment.h>
#include <AtomGroup.h>
#include <Path.h>
#include <PathGroup.h>
#include <BondTorsion.h>

struct FlagParameters *flagParameters;

void PathEntropy::init_flag_par()
	{
		flagParameters->n = 10; /* number of nearest neighbours */
		flagParameters->minres = 1e-10;
		flagParameters->kmi = 1; /* grouping of torsions within the same residue for mutual information calculations. Mutual information among groups will involve at most 2k torsions */
	}
 
Tors_res4nn* PathEntropy::get_atoms_and_residues(int numPaths, const std::vector<PathGroup> &paths, Sequence *seq)
{
    Tors_res4nn* tors_res = new Tors_res4nn[seq->size()];
    Sequence *polySeq = static_cast<Polymer *>(paths[0].front()->startInstance())->sequence();
    AtomGroup *content = paths[0].front()->startInstance()->currentAtoms();

    for (int i = 0; i < polySeq->size(); i++)
    {
        Residue *res = polySeq->residue(i);
     
        std::set<TorsionRef> torsions = res->torsions();

        int n_ang = 0;

        for (auto it = torsions.begin(); it != torsions.end(); it++)
		{
			Parameter *param = content->findParameter(it->desc(), res->id());

			if (param == nullptr)
			{
				std::cout << "NULL parameter" << std::endl;
			}
			else if (param->isTorsion() && !param->hasHydrogen()) 
			{
				BondTorsion* bondT = static_cast<BondTorsion *>(param);

    			tors_res[i].n_ang = n_ang;
				//tors_res[i].ang[n_ang]->push_back(std::vector<double>(numPaths, 0));
				tors_res[i].tors_name[n_ang] = bondT->short_desc();
                tors_res[i].desc[n_ang] = bondT->desc();
				n_ang++;
			}

		}

	}

	for (int i = 0; i < numPaths && i < paths.size(); i++)
	{

		if (!paths[i].front()->startInstance()->hasSequence())
		{
			continue;
		}

		PlausibleRoute *pr = paths[i].front()->toRoute();
		AtomGroup *content = pr->instance()->currentAtoms();
		pr->setup();

		pr->submitJobAndRetrieve(0.5, true);

		content->recalculate();

        for (int j = 0; j < polySeq->size(); j++)
        {
            for (int k = 0; k < tors_res[j].n_ang; k++)
            {
                Parameter *param = content->findParameter(tors_res[j].desc[k], polySeq->residue(j)->id());

                tors_res[j].ang[k][i] = param->empiricalMeasurement();
            }
        }
    }
	
	return tors_res;
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

int PathEntropy::calculate_entropy_independent(int nf, Sequence *seq, struct Tors_res4nn *tors_res, struct Entropy *entropy){
	int i, j, k, K, m, ok;
	double **phit;
	double *d, *ent_k, *ent_k_2, *sd_k, *ent_k_tot, *ent_k_tot_2, *d_mean, *ld_mean, *x, *y, *w, *a, *sd;
	double logdk, c, L;
	int n_res_per_model = seq->size();

	int n_tors = 0;

	entropy->n_single = n_res_per_model;
	entropy->n_nn = flagParameters->n;
	alloc_entropy(entropy, entropy->n_single, 0, entropy->n_nn, flagParameters);

	K = flagParameters->n + 1;

	d_mean = (double*)calloc(K, sizeof(double));
	ld_mean = (double*)calloc(K, sizeof(double));
	sd_k = (double*)calloc(K-1, sizeof(double));

	ent_k = (double*)calloc(K-1, sizeof(double));
	ent_k_2 = (double*)calloc(K-1, sizeof(double));
	ent_k_tot_2 = (double*)calloc(K-1, sizeof(double));

	x = (double*)calloc(K-1, sizeof(double));
	y = (double*)calloc(K-1, sizeof(double));
	w = (double*)calloc(K-1, sizeof(double));
	
	a = (double*)calloc(3, sizeof(double));
	sd = (double*)calloc(3, sizeof(double));

	for(k = 1; k <= K-1; k++)
	{
		entropy->total[k-1] = 0.0;
		entropy->sd_total[k-1] = 0.0;
		entropy->dm_total[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	for(m = 0; m < n_res_per_model; m++)
		if (tors_res[m].n_ang > 0)
		{
			phit = (double **)calloc(nf, sizeof(double *));

			for(i=0; i < nf; i++)
			{
				std::cout << tors_res[m].n_ang << std::endl;
				phit[i] = (double*)calloc(tors_res[m].n_ang, sizeof(double));
			
				for (j = 0; j < tors_res[m].n_ang; j++)
				//	if(tors_res[m].tors_name[j] == "phi");
					{
						phit[i][j] = tors_res[m].ang[j][i];
						std::cout << "Phi angle (" << i << ", " << j << "): " << phit[i][j] << std::endl;
						n_tors++;
					}

			}
	
			for(i = 0; i < K; i++)
				d_mean[i] = ld_mean[i] = 0;

			for(i = 0; i < K-1; i++)
			{
				ent_k[i] = 0;
				ent_k_2[i] = 0;
			}

		/* calculate the distance between samples in the n_ang - dimensional
	 	  space of torsion angles */

			for(i = 0; i < nf; i++)
			{
				d = (double*)calloc(nf, sizeof(double));

				for(j = 0; j < nf; j++)
				{
					d[j] = dist_ang(phit[i], phit[j], tors_res[m].n_ang);
					d[j] = deg2rad(d[j]);
				}

				/* sort the distances */
				qsort(d, nf, sizeof(double), comp);

				/* apply the entropy calculation based on the nearest neighbour */
				for(k = 1; k < K; k++)
				{
					/* if the distance is less than a pre-set value, reset the 
					 * distance to the pre-set values, to avoid NaNs */
				
					if(d[k] < flagParameters->minres)
					{
						logdk = log(flagParameters->minres);
					}
					else
					{
						logdk = log(d[k]);
					}

					ent_k_2[k-1] = ent_k_2[k-1] + logdk*logdk;
					ent_k[k-1] = ent_k[k-1] + logdk;
					d_mean[k] = d_mean[k] + d[k];
					ld_mean[k] = ld_mean[k] + logdk;
				}
				
				free(d);
			}

			for(k = 1; k < K; k++)
			{
				ent_k[k-1] = ent_k[k-1] * ((double) tors_res[m].n_ang / (double) nf);
				ent_k_2[k-1] = ent_k_2[k-1] * (double) tors_res[m].n_ang * (double) tors_res[m].n_ang / (double) nf;
			}

		for(k = 0, c = 0.0; k < tors_res[m].n_ang; k++)
			{
				c = c - log(2 * M_PI);
				c = c + ((double) tors_res[m].n_ang) * log(M_PI)/2.0 - lgamma(1.0 +  ((double) tors_res[m].n_ang)/2.0) + 0.5722 + log((double) nf);
			}

		for(k = 1, L = 0; k <= K-1; k++)
		{
			// before adding c-L compute sd
			ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
			sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
			ent_k[k-1] = ent_k[k-1] + c - L;
			L = L + 1.0/(double) k;

			d_mean[k] = d_mean[k] / (double) nf;
			ld_mean[k] = ld_mean[k] / (double) nf;

			entropy->h1[m][k-1] = ent_k[k-1];
			entropy->dm1[m][k-1] = d_mean[k];
			entropy->sd1[m][k-1] = sd_k[k-1];
			entropy->total[k-1] = entropy->total[k-1] + ent_k[k-1];
			entropy->sd_total[k-1] = entropy->sd_total[k-1] + entropy->sd1[m][k-1] * entropy->sd1[m][k-1];
			entropy->dm_total[k-1] = entropy->dm_total[k-1] + entropy->dm1[m][k-1] * entropy->dm1[m][k-1];
		}

		ok = 1;

		for(k = 0; k < K-1; k++)
		{
			x[k] = ent_k[k];
			y[k] = d_mean[k+1];

			std::cout << "standard dev: " << sd_k[k] << std::endl;

			if(sd_k[k] > 0)
				w[k] = 1/(sd_k[k] * sd_k[k]);
			else
				ok = 0;
		}

		/* if one of the weights for the linear fit is infinite,
		 * set all weights to 1 */
		if(ok == 0)
		{
			for(k = 0; k < K-1; k++)
				w[k] = 1;
		}

		fitlw(x,y,w,K-1,a,sd,&ok);

		entropy->h1lm[m] = &a[0];
		entropy->sd1lm[m] = &sd[0];
	//	entropy->dm1lm[m] = &d_mean[1]/sqrt(tors_res[m].n_ang);
		}
	
	for(k = 0; k < flagParameters->n; k++)
	{
		entropy->sd_total[k] = sqrt(entropy->sd_total[k]);
		entropy->dm_total[k] = sqrt(entropy->dm_total[k]/ (double) n_tors);
	}

}

/* Calculates entropy using mutual information for torsions closer in space than a given value */

/** int calculate_entropy_mi(int nf, struct Tors_res4nn *tors_res, struct Tors_res4nn **tors_mi, struct Flag_par flag_par, struct Entropy *entropy)
{
	int i, j, K, l, m, n_res_per_model_mi, *group2res;
	double **phit;
	struct Tors_res4nn tors_mi2;

	// for each residue...
	K = flag_par.n + 1;
	// tors_res2mi(&n_res_per_model_mi, &group2res, flag_par);

	//... based on a cutoff distance, calculate how many pairs of groups must be considered
	(*entropy).n_single = n_res_per_model_mi;
	(*entropy).n_pair = 0;
	for(i = 0; i < n_res_per_model_mi; i++)
		for(j = i+1; j < n_res_per_model_mi; j++)
		{
			for(l = 0; l < (*tors_mi)[i].n_ang; l++)
			for(m = 0; m < (*tors_mi)[j].n_ang; m++)
				if(distv((tors_mi)[i].v[l], (tors_mi)[j].v[m]) <= flag_par.cutoff)
				{
					l = (*tors_mi)[i].n_ang + 1;
					m = (*tors_mi)[j].n_ang + 1;
					(*entropy).n_pair++;
				}
		}
	
	alloc_entropy(entropy, n_res_per_model_mi, (*entropy).n_pair, (*entropy).n_nn, flag_par);
	
	phit = (double*) calloc(nf, sizeof(double*));

	// for each group, compute entropy, sd and dm for the group and map to residues
	 // sum to total entrop, sd and dm
	 // normalise dm by sqrt(dof)
	for(m = 0; m < n_res_per_model_mi; m++)
	{
		for(i=0; i < nf; i++)
		{
			phit[i] = (double) calloc((*tors_mi)[m].n_ang, sizeof(double));
		}
	}

} */

/* int tors_res2mi(int *n_res_per_model_mi, struct Tors_res4nn **tors_mi, int **group2res,  struct Flag_par flag_par)
{
	int i, j, k, l;

	l = 0;

	for(i = 0; i < seq.size(); i++)
	{
		l = l + (int) floor((double) torsionAngles[i] / (double) flag_par.kmi);
	//	if((torsionAngles[i] % flag_par.kmi) !=0) l++; 
	}

	*n_res_per_model_mi = l;

	(*tors_mi) = (Tors_res4nn*) calloc(*n_res_per_model_mi, sizeof(Tors_res4nn));
	(*group2res) = (int*) calloc(*n_res_per_model_mi, sizeof(int));

	for(i = 0; i < *n_res_per_model_mi; i++)
	{
		(*tors_mi)[i].phi = (double**) calloc(flag_par.kmi, sizeof(double *));
	}

	// group torsions
	for(i = 0, j = 0; i < seq.size(); i++)
	{
		for(k = 0; k < 4; k++)
			(*tors_mi)[j].phi[k % flag_par.kmi] = tors_res[i].phi[k];
			(*tors_mi)[j].n_ang = k % flag_par.kmi + 1;

		if((k % flag_par.kmi) == (flag_par.kmi - 1) || k == (tors_res[i].n_ang - 1))
		{
			(*group2res)[j] = i;
			j++;
		}
	}

}*/

int PathEntropy::comp(const void* elem1, const void* elem2){
	double f1 = *((double *)elem1);
	double f2 = *((double *)elem2);
	if (f1 > f2) return 1;
	if (f1 < f2) return -1;
	return 0;
}

/* linear weighting function */
int PathEntropy::fitlw(double *x, double *y, double *w, int n, double *a, double *sd, int *ok)
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

int PathEntropy::alloc_tors(struct Tors_res4nn *tors_res, int seqSize)
{
	tors_res = (struct Tors_res4nn*) calloc(seqSize, sizeof(struct Tors_res4nn));
}

/* allocates memory to entropy structure */
int PathEntropy::alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct FlagParameters *flagParameters)
{
	int i;

	entropy->total = (double*) calloc(n_nn, sizeof(double));
	entropy->sd_total = (double*) calloc(n_nn, sizeof(double));
	entropy->dm_total = (double*) calloc(n_nn, sizeof(double));
	entropy->h1lm = (double**) calloc(n_single, sizeof(double));
	entropy->sd1lm = (double**) calloc(n_single, sizeof(double));
	entropy->sd1 = (double**) calloc(n_single, sizeof(double*));
	entropy->dm1 = (double**) calloc(n_single, sizeof(double*));

	for(i = 0; i < n_single; i++)
	{
		entropy->h1[i] = (double*) calloc(n_nn, sizeof(double));
		entropy->sd1[i] = (double*) calloc(n_nn, sizeof(double));
		entropy->dm1[i] = (double*) calloc(n_nn, sizeof(double));
	}
}

