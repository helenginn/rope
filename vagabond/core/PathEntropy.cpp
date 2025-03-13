#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <math.h>
#include <../utils/degrad.h>
#include <matrix_functions.h>
#include <PathEntropy.h>
#include <Sequence.h>
#include <Model.h>
#include <Residue.h>
#include <Environment.h>
#include <ModelManager.h>
#include <AtomGroup.h>
#include <BondTorsion.h>
#include <HasBondstraints.h>

BondTorsion *torsion;
Sequence seq;
double *torsionAngles;
std::vector<double> torsionMeasurements;
struct Tors_res4nn *tors_res;

void PathEntropy::init_flag_par(struct Flag_par *flag_par)
	{
		(*flag_par).n = 10; /* number of nearest neighbours */
		(*flag_par).minres = 1e-10;
		(*flag_par).kmi = 1; /* grouping of torsions within the same residue for mutual information calculations. Mutual information among groups will involve at most 2k torsions */
	}
 
void PathEntropy::get_atoms_and_residues(const std::string &model_id)
{
	ModelManager *mm = Environment::modelManager();
	Model *const model = mm->model(model_id);
	model->load();

	std::vector<Instance *> instances = model->instances();

	for (int i = 0; i < instances.size(); i++)
	{
		if (instances[i]->hasSequence())
		{
			AtomGroup *content = instances[i]->currentAtoms();
			content->recalculate();

			Atom *atom = (*content)[0];
			Sequence seq = Sequence(atom);

			for (int j = 0; j < seq.size(); j++)
			{
				Residue *res = seq.residue(j);
				std::set<TorsionRef> torsions = res->torsions();

				for (auto it = torsions.begin(); it != torsions.end(); it++)
				{
					Parameter *param = content->findParameter(it->desc(), res->id());

					if (param->isTorsion())
					{
						static_cast<BondTorsion *>(param);
					}
				}
			}
		}
	}

	// loop around model->instances()
	// // check if instance->hasSequence()
	// // // if so, loop round sequence->size()...
	// // // // Residue -> residueId
	// // // // Residue -> torsionRefs (loop around these)
	// // // // // torsionRef + residueId in findParameter(instance->currentAtoms()) to get Parameter
	// // // // // Parameter check isTorsion(), if so static_cast<BondTorsion *>(parameter)
	// // // // // check if you want to include it (hydrogens?) - if so, count++
	//
	

	int n_res_per_model = seq.size();
	
	/* tors_res = (struct Tors_res4nn*)calloc(n_res_per_model, sizeof(struct Tors_res4nn));

	for(int j = 0; j < n_res_per_model; j++)
	{
		tors_res[j].phi = (double*)calloc(seq.residue(j)->torsionCount(), sizeof(double));

		for(int k = 0; k < seq.residue(k)->torsionCount(); k++)
		{
			tors_res[j].n_ang = 0;
			tors_res[j].tors_name = content->bondTorsion(j+k)->short_desc();
			tors_res[j].phi[tors_res[j].n_ang] = content->bondTorsion(j+k)->measurement(BondTorsion::SourceDerived, true);
			tors_res[j].n_ang++;
		}
		
	}*/

	/*for(int j = 0; j < content->bondTorsionCount(); j++)
	{
		BondTorsion *torsion = content->bondTorsion(j);

		if (torsion->measurement(BondTorsion::SourceDerived, true) < 0)
		{
			torsionMeasurements.push_back(torsion->measurement(BondTorsion::SourceDerived, true));
		}
	}*/
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

int PathEntropy::calculate_entropy_independent(int nf, struct Flag_par flag_par, struct Entropy *entropy){
	int i, j, k, K, m, ok;
	double **phit;
	double *d, *ent_k, *ent_k_2, *sd_k, *ent_k_tot, *ent_k_tot_2, *d_mean, *ld_mean, *x, *y, *w, *a, *sd;
	double logdk, c, L;
	int n_res_per_model = seq.size();
	int n_tors = sizeof(tors_res->n_ang)/sizeof(int);

	std::cout << n_res_per_model << " " << n_tors << std::endl;

	(*entropy).n_single = n_res_per_model;
	(*entropy).n_nn = flag_par.n;
	alloc_entropy(entropy, n_tors, 0, (*entropy).n_nn, flag_par);

	K = flag_par.n + 1;

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
		(*entropy).total[k-1] = 0.0;
		(*entropy).sd_total[k-1] = 0.0;
		(*entropy).dm_total[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	// std::cout << Tors_res4nn[0]->refinedAngle() << std::endl;

	for(m = 0; m < n_res_per_model; m++)
		if (tors_res[m].n_ang > 0)
		{
			phit = (double **)calloc(1, sizeof(double *));
			phit[0] = (double*)calloc(1, sizeof(double));
			for (j = 0; j < tors_res[m].n_ang; j++)
			{
				phit[0][j] = tors_res[m].phi[j];
			}
	
			d = (double*)calloc(1, sizeof(double));

			for(i = 0; i < K; i++)
				d_mean[i] = ld_mean[i] = 0;

			for(i = 0; i < K-1; i++)
			{
				ent_k[i] = 0;
				ent_k_2[i] = 0;
			}

		/* calculate the distance between samples in the n_ang - dimensional
	 	  space of torsion angles */

			for(j = 0; j < nf; j++)
  	 		{
				d[j] = dist_ang(phit[0], phit[j], tors_res[m].n_ang);
				d[j] = deg2rad(d[j]);
				std::cout << d[j] << std::endl;
			}

			/* sort the distances */
			qsort(d, nf, sizeof(double), comp);

			/* apply the entropy calculation based on the nearest neighbour */
			for(k = 1; k < K; k++)
			{
				/* if the distance is less than a pre-set value, reset the 
				 * distance to the pre-set values, to avoid NaNs */
			
				if(d[k] < flag_par.minres)
				{
					logdk = log(flag_par.minres);
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

			for(k = 1; k < K; k++)
			{
				ent_k[k-1] = ent_k[k-1] * 1;
				ent_k_2[k-1] = ent_k_2[k-1] * 1 * 1;
			}

		for(k = 0, c = 0.0; k < 1; k++)
			{
				c = c - log(2 * M_PI);
				c = c + 1.0 * log(M_PI)/2.0 - lgamma(1.0 +  1/2.0) + 0.5722;
			}

		for(k = 1, L = 0; k <= K-1; k++)
		{
			// before adding c-L compute sd
			ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
			sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
			ent_k[k-1] = ent_k[k-1] + c - L;
			L = L + 1.0/(double) k;

		//	(*entropy).h1[m][k-1] = ent_k[k-1];
			(*entropy).dm1[m][k-1] = d_mean[k];
			(*entropy).sd1[m][k-1] = sd_k[k-1];
			(*entropy).total[k-1] = (*entropy).total[k-1] + ent_k[k-1];
			(*entropy).sd_total[k-1] = (*entropy).sd_total[k-1] + (*entropy).sd1[m][k-1] * (*entropy).sd1[m][k-1];
			(*entropy).dm_total[k-1] = (*entropy).dm_total[k-1] + (*entropy).dm1[m][k-1] * (*entropy).dm1[m][k-1];
		}

		ok = 1;

		for(k = 0; k < K-1; k++)
		{
			x[k] = ent_k[k];
			y[k] = d_mean[k+1];

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

		(*entropy).h1lm[m] = &a[0];
		(*entropy).sd1lm[m] = &sd[0];
		// (*entropy).dm1lm[m] = d_mean[1]/sqrt((double) torsionAngles[m]);
		}
	
	for(k = 0; k < flag_par.n; k++)
	{
		(*entropy).sd_total[k] = sqrt((*entropy).sd_total[k]);
		(*entropy).dm_total[k] = sqrt((*entropy).dm_total[k]/ (double) n_tors);
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

int PathEntropy::comp(const void * elem1, const void * elem2){
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

/* allocates memory to entropy structure */
int PathEntropy::alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct Flag_par flag_par)
{
	int i;

	(*entropy).total = (double*) calloc(n_nn, sizeof(double));
	(*entropy).sd_total = (double*) calloc(n_nn, sizeof(double));
	(*entropy).dm_total = (double*) calloc(n_nn, sizeof(double));
	(*entropy).h1lm = (double**) calloc(n_single, sizeof(double));
	(*entropy).sd1lm = (double**) calloc(n_single, sizeof(double));
	(*entropy).sd1 = (double**) calloc(n_single, sizeof(double*));
	(*entropy).dm1 = (double**) calloc(n_single, sizeof(double*));

	for(i = 0; i < n_single; i++)
	{
	//	(*entropy).h1[i] = (double*) calloc(n_nn, sizeof(double));
		(*entropy).sd1[i] = (double*) calloc(n_nn, sizeof(double));
		(*entropy).dm1[i] = (double*) calloc(n_nn, sizeof(double));
	}
}

