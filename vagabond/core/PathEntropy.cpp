#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <numeric>
#include <math.h>
#include <../utils/degrad.h>
#include <../utils/glm_import.h>
#include <../c4x/Cluster.h>
#include <matrix_functions.h>
#include <PathEntropy.h>
#include <Sequence.h>
#include <TorsionRef.h>
#include <FixIssues.h>
#include <Residue.h>
#include <AtomGroup.h>
#include <Path.h>
#include <PathGroup.h>
#include <BondTorsion.h>

struct FlagParameters flagParameters = {0};

void PathEntropy::init_flag_par()
	{
	    flagParameters.n = 10; /* number of nearest neighbours */
		flagParameters.minres = 1e-10;
		flagParameters.kmi = 1; /* grouping of torsions within the same residue for mutual information calculations. Mutual information among groups will involve at most 2k torsions */
	}
 
std::vector<Tors_res4nn*> PathEntropy::get_atoms_and_residues(int numPaths, const std::vector<PathGroup> &paths, Sequence *seq)
{
    std::vector<Tors_res4nn*> tors_res(seq->size());
    Sequence *polySeq = static_cast<Polymer *>(paths[0].front()->startInstance())->sequence();
    
    AtomGroup *content = paths[0].front()->toRoute()->instance()->currentAtoms();

    content->recalculate();

    for (int i = 0; i < polySeq->size(); i++)
    {
        Residue *res = polySeq->residue(i);
     
        std::set<TorsionRef> torsions = res->torsions();

        std::vector<BondTorsion*> valid_bondT;
 
        for (auto it = torsions.begin(); it != torsions.end(); it++)
		{
			Parameter *param = content->findParameter(it->desc(), res->id());

			if (param == nullptr)
			{
				std::cout << "NULL parameter" << std::endl;
			}
			else if (param->isTorsion() && !param->hasHydrogen()) 
			{
                valid_bondT.push_back(static_cast<BondTorsion *>(param));
            }
       }

        tors_res[i] = new Tors_res4nn;
        tors_res[i]->tors_name.resize(valid_bondT.size());
        tors_res[i]->desc.resize(valid_bondT.size());

        tors_res[i]->n_ang = valid_bondT.size();

        for(int j = 0; j < valid_bondT.size(); j++)
        {
            tors_res[i]->ang.push_back(std::vector<double>(numPaths, 0));
            tors_res[i]->v.push_back(std::vector<glm::vec3>(numPaths));
            tors_res[i]->tors_name[j] = valid_bondT[j]->short_desc();
            tors_res[i]->desc[j] = valid_bondT[j]->desc();

            if((tors_res[i]->desc[j] == "chi2" && (res->code() == "ASP"|| res->code() == "PHE" || res->code() == "TYR")) ||
            (tors_res[i]->desc[j] == "chi3" && res->code() == "GLU") ||
            (tors_res[i]->desc[j] == "chi4" && res->code() == "ARG"))
            {
                tors_res[i]->bondSymmetry = 2;
            }
            else
            {
                tors_res[i]->bondSymmetry = 1;
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
        pr->setup();
        pr->submitJobAndRetrieve(0.05, true);

		AtomGroup *content = pr->instance()->currentAtoms();
		//content->recalculate();

        for (int j = 0; j < polySeq->size(); j++)
        {
            for (int k = 0; k < tors_res[j]->n_ang; k++)
            {
                Parameter *param = content->findParameter(tors_res[j]->desc[k], polySeq->residue(j)->id());

                tors_res[j]->ang[k][i] = param->empiricalMeasurement();
                tors_res[j]->v[k][i] = param->owningAtom()->derivedPosition();
            }
        }
    }
	
	return tors_res;
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

struct Entropy* PathEntropy::calculate_entropy_independent(int nf, Sequence *seq, std::vector<Tors_res4nn*> tors_res){
	int i, j, k, ok;
	double **phit;
	double *x, *y, *w, *a, *sd, *ent_k, *ent_k_2, *ent_k_tot, *ent_k_tot_2;
	double logdk, c, L;
	int n_res_per_model = tors_res.size();

	int n_tors = 0;

    struct Entropy* entropy = new Entropy;

	entropy->n_single = n_res_per_model;
	entropy->n_nn = flagParameters.n;
	alloc_entropy(entropy, entropy->n_single, 0, entropy->n_nn, flagParameters);

	int K = flagParameters.n + 1;

    double d[nf] = {};
	double d_mean[K] = {};
	double ld_mean[K] = {};
	double sd_k[K-1] = {};

	ent_k, ent_k_2, ent_k_tot_2 = new double[K-1];

	x, y, w = new double[K-1];

    a, sd = new double[3];	

	for(k = 1; k <= K-1; k++)
	{
		entropy->total[k-1] = 0.0;
		entropy->sd_total[k-1] = 0.0;
		entropy->dm_total[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	for(int m = 0; m < n_res_per_model; m++)
		if (tors_res[m]->n_ang > 0)
		{
			phit = new double *[nf];

			for(i=0; i < nf; i++)
			{
				std::cout << tors_res[m]->n_ang << std::endl;
				phit[i] = new double[tors_res[m]->n_ang];
			
				for (j = 0; j < tors_res[m]->n_ang; j++)
				    if(tors_res[m]->tors_name[j] == "phi");
					{
						phit[i][j] = tors_res[m]->ang[j][i];
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
				for(j = 0; j < nf; j++)
				{
					d[j] = dist_ang(phit[i], phit[j], tors_res[m]->n_ang);
					d[j] = deg2rad(d[j]);
				}

				/* sort the distances */
				qsort(d, nf, sizeof(double), comp);

				/* apply the entropy calculation based on the nearest neighbour */
				for(k = 1; k < K; k++)
				{
					/* if the distance is less than a pre-set value, reset the 
					 * distance to the pre-set values, to avoid NaNs */
				
					if(d[k] < flagParameters.minres)
					{
						logdk = log(flagParameters.minres);
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
			}

			for(k = 1; k < K; k++)
			{
				ent_k[k-1] = ent_k[k-1] * ((double) tors_res[m]->n_ang / (double) nf);
				ent_k_2[k-1] = ent_k_2[k-1] * (double) tors_res[m]->n_ang * (double) tors_res[m]->n_ang / (double) nf;
			}

		for(k = 0, c = 0.0; k < tors_res[m]->n_ang; k++)
			{
				c = c - log(2 * M_PI);
				c = c + ((double) tors_res[m]->n_ang) * log(M_PI)/2.0 - lgamma(1.0 +  ((double) tors_res[m]->n_ang)/2.0) + 0.5722 + log((double) nf);
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

		entropy->h1lm[m] = a[0];
		entropy->sd1lm[m] = sd[0];
	//	entropy->dm1lm[m] = &d_mean[1]/sqrt(tors_res[m].n_ang);
		}
	
	for(k = 0; k < flagParameters.n; k++)
	{
		entropy->sd_total[k] = sqrt(entropy->sd_total[k]);
		entropy->dm_total[k] = sqrt(entropy->dm_total[k]/ (double) n_tors);
	}

    return entropy;

}

/* Calculates entropy using mutual information for torsions closer in space than a given value */

struct Entropy* PathEntropy::calculate_entropy_mi(int nf, std::vector<Tors_res4nn*> tors_res, struct FlagParameters flagParameters)
{
	int i, j, k, ii, jj, kk, K, l, m, ok, *group2res;
	double **phit;
    double *ent_k, *ent_k_tot, *ent_k_2, *sd_k, *ent_k_tot_2, *d_mean, *ld_mean, *x, *y, *w, *a, *sd, *d, logdk, c, L;

    int n_res_per_model, n_res_per_model_mi = tors_res.size();
    std::vector<Tors_res4nn*> tors_mi(n_res_per_model_mi);
    
    struct Tors_res4nn tors_mi2;
    struct Entropy *entropy = new Entropy;

	// for each residue...
	K = flagParameters.n + 1;
	tors_res2mi(tors_res, n_res_per_model, tors_mi, n_res_per_model, group2res, flagParameters);

    ent_k, ent_k_2, ent_k_tot, ent_k_tot_2 = new double[K-1];

	//... based on a cutoff distance, calculate how many pairs of groups must be considered
	entropy->n_single = n_res_per_model_mi;
	entropy->n_pair = 0;
    entropy->n_nn = flagParameters.n;
 
	for(i = 0; i < n_res_per_model_mi; i++)
		for(j = i+1; j < n_res_per_model_mi; j++)
		{
			for(l = 0; l < tors_mi[i]->n_ang; l++)
			for(m = 0; m < tors_mi[j]->n_ang; m++)
            {
				if(glm::length(tors_mi[i]->v[l]-tors_mi[j]->v[m]) <= flagParameters.cutoff)
				{
					l = tors_mi[i]->n_ang + 1;
					m = tors_mi[j]->n_ang + 1;
					entropy->n_pair++;
				}
            }
		}
	
    //alloc_entropy(entropy, n_res_per_model_mi, entropy->n_pair, entropy->n_nn, flagParameters);
	
	//phit = (double*) calloc(nf, sizeof(double*));

	// for each group, compute entropy, sd and dm for the group and map to residues
	 // sum to total entrop, sd and dm
	 // normalise dm by sqrt(dof)
	for(m = 0; m < n_res_per_model_mi; m++)
	{
		for(i = 0; i < nf; i++)
		{
			phit[i] = new double[tors_mi[m]->n_ang];
            
            for(j = 0; j < tors_mi[m]->n_ang; j++)
            {
                phit[i][j] = tors_mi[m]->ang[j][i];
            }
		}

        for(i = 0; i < K-1; i++)
        {
            ent_k_2[i] = ent_k[i] = 0;
        }

        for(i = 0; i < K; i++)
        {
            d_mean[i] = ld_mean[i] = 0;
        }
	
        for(i = 0; i < nf; i++)
        {
	 	    d = new double[nf];
		
            for(j = 0; j < nf; j++)
		    {
		        d[j] = dist_ang(phit[i], phit[j], tors_mi[m]->n_ang);
		        d[j] = d[j] * M_PI/180.0;
		    }
        }
	    
        qsort(d,nf,sizeof(double), comp);

		for(k = 1; k < K; k++)
		{
		    if(d[k] < flagParameters.minres)
            {
		        logdk = log(flagParameters.minres);
            }
		    else
            {
		        logdk = log(d[k]);
            }
		
			ent_k[k-1] = ent_k[k-1] + logdk ;
			ent_k_2[k-1] = ent_k_2[k-1] + logdk*logdk;
			d_mean[k] = d_mean[k] + d[k];
			ld_mean[k] = ld_mean[k] + logdk;	
		}

		for(k = 1; k < K; k++)
		{
			ent_k[k-1] = ent_k[k-1] * ((double) tors_mi[m]->n_ang / (double) nf);
			ent_k_2[k-1] = ent_k_2[k-1] * (double) tors_mi[m]->n_ang * (double) tors_mi[m]->n_ang / (double) nf;
		}

		for(k = 0, c = 0.0; k < tors_mi[m]->n_ang; k++)
        {
			c = c - log(M_PI /180.0);
			c = c + (double) tors_mi[m]->n_ang * log(M_PI)/2.0 - lgamma(1.0 + (double) tors_mi[m]->n_ang)/2.0 + 0.5722 + log((double) nf);
		}
		
        for(k = 1, L = 0; k < K; k++)
		{
		    // before adding c-L compute sd
		    ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
		    sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
		    // add c-L 
			ent_k[k-1] = ent_k[k-1] + c - L;
			L = L + 1.0/(double) k;
			d_mean[k] = d_mean[k]/(double) nf;
			ld_mean[k] = ld_mean[k]/(double) nf;
			entropy->h1[m][k-1] = ent_k[k-1];
			entropy->dm1[m][k-1] = d_mean[k];
			entropy->sd1[m][k-1] = sd_k[k-1];
		}
	
     	// Linear weighted fit
		ok = 1;
		
        for(k = 0; k < K-1; k++)
		{
			x[k] = d_mean[k+1];
			y[k] = ent_k[k];
			
            if(sd_k[k] > 0) w[k] = 1/(sd_k[k] * sd_k[k]);
			else ok = 0;
		}
		
        if(ok == 0)
		{
			for(k=0; k< K-1; k++)
			w[k] = 1;
		}
	
    	fitlw(y,x,w,K-1,a,sd,&ok);
		
		entropy->h1lm[m] = a[0];
		entropy->sd1lm[m] = sd[0] * sd[0];
		entropy->dm1lm[m] = d_mean[1] * d_mean[1];
	}
	
    // ... then prepare for mutual information calculation ... 
	tors_mi2.ang.resize(2*flagParameters.kmi);
	
    phit = new double *[nf];
	
    for(i = 0; i < nf; i++)
    {
	    phit[i] = new double[2*flagParameters.kmi];
    }

	kk = 0;
	// ... and calculate the entropy for paired groups of torsions 
	// with the nearest neighbour method ... 
	for(ii = 0; ii < n_res_per_model_mi; ii++)
	for(jj = ii + 1; jj < n_res_per_model_mi; jj++)
	for(l = 0; l < tors_mi[ii]->n_ang; l++)
	for(m = 0; m < tors_mi[jj]->n_ang; m++)
		if(glm::length(tors_mi[ii]->v[l] - tors_mi[jj]->v[m]) <= flagParameters.cutoff)
		{
			entropy->i1[kk] = ii;
			entropy->i2[kk] = jj;

			l = tors_mi[ii]->n_ang + 1;
			m = tors_mi[jj]->n_ang + 1;
			tors_mi2.n_ang = tors_mi[ii]->n_ang + tors_mi[jj]->n_ang;

			for(k = 0,i = 0; k < tors_mi[ii]->n_ang; k++)
			{
			    tors_mi2.ang[i] = tors_mi[ii]->ang[k];
			}
			
            for(k = 0; k < tors_mi[jj]->n_ang; k++)
			{
			    tors_mi2.ang[i] = tors_mi[jj]->ang[k];
		    }
        
			for(i = 0; i < nf; i++)
			for(j = 0; j < tors_mi2.n_ang; j++)
			    phit[i][j] = tors_mi2.ang[j][i];

			for(i = 0; i < K-1; i++)
            {
			    ent_k_2[i] = ent_k[i] = 0.0;
            }
			
            for(i = 0; i<K; i++)
            {
			    d_mean[i] = ld_mean[i] = 0;
            }
			
			d = new double[nf];
			
            for(j = 0; j < nf; j++)
			{
				d[j] = dist_ang(phit[i],phit[j],tors_mi2.n_ang);
				d[j] = deg2rad(d[j]);
			}

			qsort(d, nf, sizeof(double), comp);

			for(k = 1; k < K; k++)
			{
			    if(d[k] < flagParameters.minres)
                {
			        logdk = log(flagParameters.minres);
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

			for(k = 1; k<K; k++)
			{
				ent_k[k-1] = ent_k[k-1] * ((double) tors_mi2.n_ang / (double) nf);
				ent_k_2[k-1] = ent_k_2[k-1] * ((double) tors_mi2.n_ang * (double) tors_mi2.n_ang/ (double) nf);
			}

			for(k = 0,c = 0.0; k < tors_mi2.n_ang; k++)
            {
				c = c - log(M_PI/180.0);
				c = c + ((double) tors_mi2.n_ang) * log(M_PI)/2.0 - lgamma(1.0 + ((double) tors_mi2.n_ang)/2.0) + 0.5722 + log( (double) nf);

			for(k = 1, L = 0; k <= K - 1; k++)
			{
				// before adding c-L compute sd
				ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
				sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
				ent_k[k-1] = ent_k[k-1] + c - L;
				L = L + 1.0/(double) k;
				d_mean[k] = d_mean[k]/(double) nf;
				ld_mean[k] = ld_mean[k]/(double) nf;
				ent_k_tot[k-1] = ent_k_tot[k-1] + ent_k[k-1];
			}
			
            //... compute, by subtraction of single group entropies, the mutual information ...
			for(k = 0; k < K - 1; k++)
			{
				entropy->h2[kk][k] = ent_k[k];
				entropy->sd2[kk][k] = sd_k[k];
				entropy->dm2[kk][k] = d_mean[k];
				entropy->mi[kk][k] = entropy->h2[kk][k] - entropy->h1[entropy->i1[kk]][k] - entropy->h1[entropy->i2[kk]][k];
				entropy->sdmi[kk][k] =  pow(entropy->sd2[kk][k],2) + pow(entropy->sd1[entropy->i1[kk]][k],2) + pow(entropy->sd1[entropy->i2[kk]][k],2);
				entropy->dmmi[kk][k] = pow(entropy->dm2[kk][k],2) + pow(entropy->dm1[entropy->i1[kk]][k],2) + pow((*entropy).dm1[entropy->i2[kk]][k],2);
			}
			
            // linear weighted fit
			ok = 1;
			
            for(k = 0; k < K - 1; k++)
			{
				x[k] = d_mean[k+1];
				y[k] = ent_k[k];
				
                if(k == 0) w[k] = M_PI * M_PI /6;
				else w[k] = w[k-1] - 1/(double) (k*k);
			}

			fitlw(y,x,w,K-1,a,sd,&ok);

			entropy->h2lm[kk] = a[0]; 
			entropy->sd2lm[kk] = sd[0]; 
			entropy->dm2lm[kk] = d_mean[1];
			entropy->milm[kk] = entropy->h2lm[kk] - entropy->h1lm[entropy->i1[kk]] - entropy->h1lm[entropy->i2[kk]]; 
			kk++;
		}
	}

    return entropy;

} 

int PathEntropy::tors_res2mi(std::vector<Tors_res4nn*> tors_res, int n_res_per_model, std::vector<Tors_res4nn*> tors_mi, int n_res_per_model_mi, int *group2res, struct FlagParameters flagParameters)
{
	int i, j, k, l;

	l = 0;

	for(i = 0; i < n_res_per_model; i++)
	{
		l = l + (int) floor((double) tors_res[i]->n_ang / (double) flagParameters.kmi);

		if((tors_res[i]->n_ang % flagParameters.kmi) !=0) l++; 
	}

	n_res_per_model_mi = l;

	group2res = new int[n_res_per_model_mi];

	for(i = 0; i < n_res_per_model_mi; i++)
	{
		tors_mi[i]->ang.resize(flagParameters.kmi);
        tors_mi[i]->v.resize(flagParameters.kmi);
        tors_mi[i]->tors_name.resize(flagParameters.kmi);
	}

	// group torsions
	for(i = 0, j = 0; i < n_res_per_model_mi; i++)
	{
		for(k = 0; k < tors_res[i]->n_ang; k++)
        {
		    tors_mi[j]->ang[k % flagParameters.kmi] = tors_res[i]->ang[k];
            tors_mi[j]->v[k % flagParameters.kmi] = tors_res[i]->v[k];
			tors_mi[j]->n_ang = k % flagParameters.kmi + 1;
        }

		if((k % flagParameters.kmi) == (flagParameters.kmi - 1) || k == (tors_res[i]->n_ang - 1))
		{
			group2res[j] = i;
			j++;
		}
	}
}

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
int PathEntropy::alloc_entropy(struct Entropy *entropy, int n_single, int n_pair, int n_nn, struct FlagParameters flagParameters)
{
	entropy->total = new double[n_nn];
	entropy->sd_total = new double[n_nn];
	entropy->dm_total = new double[n_nn];
	entropy->h1lm = new double [n_single];
	entropy->sd1lm = new double[n_single];
    entropy->h1 = new double *[n_single];
	entropy->sd1 = new double *[n_single];
	entropy->dm1 = new double *[n_single];

	for(int i = 0; i < n_single; i++)
	{
		entropy->h1[i] = new double[n_nn];
		entropy->sd1[i] = new double[n_nn];
		entropy->dm1[i] = new double[n_nn];
	}

    if(flagParameters.mutualInformation)
    {
        entropy->mi = new double*[entropy->n_pair];
    }

}

