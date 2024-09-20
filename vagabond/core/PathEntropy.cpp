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
#include <Environment.h>
#include <ModelManager.h>
#include <AtomGroup.h>
#include <BondTorsion.h>
#include <HasBondstraints.h>


double *torsionAngles;

Sequence seq;
BondTorsion *torsion;

void init_flag_par(struct Flag_par *flag_par);
void get_atoms_and_residues(const std::string &model_id);

void get_atoms_and_residues(const std::string &model_id)
{
	ModelManager *mm = Environment::modelManager();

	Model *const model = mm->model(model_id);

	model->load();
	AtomGroup *content = model->currentAtoms();
	content->recalculate();

	Atom *atom = (*content)[0];

	for(int i = 0; i < content->bondTorsionCount(); i++)
	{
		BondTorsion *torsion = content->bondTorsion(i);
	}

	double torsionAngles = torsion->measurement(BondTorsion::SourceDerived, true);
	
	
	Sequence seq = Sequence(atom);
} 


/* Calculates entropy from torsion angles, assuming independence between the residues */

int calculate_entropy_independent(int nf, struct Flag_par flag_par, struct Entropy *entropy){
	int i, j, k, K, m, ok;
	double **phit;
	double *d, *ent_k, *ent_k_2, *sd_k, *ent_k_tot, *ent_k_tot_2, *d_mean, *ld_mean, *x, *y, *w, *a, *sd;
	double logdk, c, L;
	int n_res_per_model = seq.size();
	int n_tors = *(&torsionAngles + 1) - torsionAngles;

	(*entropy).n_single = n_res_per_model;
	(*entropy).n_nn = flag_par.n;
	K = flag_par.n + 1;

	for(k = 1; k <= K-1; k++)
	{
		(*entropy).total[k-1] = 0.0;
		(*entropy).sd_total[k-1] = 0.0;
		(*entropy).dm_total[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	for(m = 0; m < n_res_per_model; m++)
		if (torsionAngles[m] > 0)
		{
		phit = (double **)calloc(1, sizeof(double *));
		
		for(j = 0; j < n_tors; j++)
			phit[1][j] = torsionAngles[m];
		

		for(i = 0; i < K; i++)
			d_mean[i] = ld_mean[i] = 0;

		for(i = 0; i < K-1; i++)
			{
				ent_k[i] = 0;
				ent_k_2[i] = 0;
			}

		/* calculate the distance between samples in the n_ang - dimensional
	 	  space of torsion angles */

		d = (double*)calloc(1, sizeof(double));

		for(j = 0; j < nf; j++)
		{
			d[j] = dist_ang(phit[0], phit[j], 1);
			d[j] = deg2rad(d[j]);
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

		for(k = 1; k < K; k++)
		{
			ent_k[k-1] = ent_k[k-1] * (double) torsionAngles[m];
			ent_k_2[k-1] = ent_k_2[k-1] * (double) torsionAngles[m] * (double) torsionAngles[m];
		}

		for(k = 0, c = 0.0; k < torsionAngles[m]; k++)
		{
			c = c - log(2 * M_PI);
			c = + ((double) torsionAngles[m]) * log(M_PI)/2.0 - lgamma(1.0 + ((double) torsionAngles[m])/2.0) + 0.5722;
		}

		for(k = 1; k <= K-1; k++)
		{
			// before adding c-L compute sd
			ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
			sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
			ent_k[k-1] = ent_k[k-1] + c - L;
			L = L + 1.0/(double) k;

			(*entropy).h1[m][k-1] = ent_k[k-1];
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

		(*entropy).hd1lm[m] = &a[0];
		(*entropy).sd1lm[m] = &sd[0];
		// (*entropy).dm1lm[m] = d_mean[1]/sqrt((double) torsionAngles[m]);

		for(k = 0; k < flag_par.n; k++)
		{
			(*entropy).sd_total[k] = sqrt((*entropy).sd_total[k]);
			(*entropy).dm_total[k] = sqrt((*entropy).dm_total[k]);
		}
	}

}

