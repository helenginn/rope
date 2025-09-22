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
#include <Residue.h>
#include <AtomGroup.h>
#include <Path.h>
#include <PathGroup.h>
#include <BondTorsion.h>

struct FlagParameters PathEntropy::initFlagPar()
{
    struct FlagParameters flagParameters;

	flagParameters.n = 10; /* number of nearest neighbours */
    flagParameters.ne = 5;
	flagParameters.minres = 1e-10;
    flagParameters.cutoff = 6.0;
	flagParameters.kmi = 1; /* grouping of torsions within the same residue for mutual information calculations. Mutual information among groups will involve at most 2k torsions */

    return flagParameters;
}

std::vector<TorsRes4NN*> PathEntropy::getAtomsAndResidues(int numPaths, const std::vector<PathGroup> &paths)
{
	std::vector<TorsRes4NN*> tors_res;
    Sequence *polySeq = static_cast<Polymer *>(paths[0].front()->startInstance())->sequence();

	AtomGroup *content = paths[0].front()->toRoute()->instance()->currentAtoms();

	content->recalculate();

	for (int i = 0; i < polySeq->size(); i++)
	{
		if(polySeq->residue(i) == nullptr)
		{
			break;
		}

        Residue *res = polySeq->residue(i);

		std::set<TorsionRef> torsions = res->torsions();

		std::vector<BondTorsion*> valid_bondT;
        std::vector<std::string> angNames;

		for (auto it = torsions.begin(); it != torsions.end(); it++)
		{
			Parameter *param = content->findParameter(it->desc(), res->id());

			if (param == nullptr || !param->isTorsion() || param->hasHydrogen())
			{
				std::cout << res->id() << " invalid parameter" << std::endl;
                continue;
			}
            if (angNames.size() > 0 && std::find(angNames.begin(), angNames.end(), static_cast<BondTorsion *>(param)->short_desc()) != angNames.end())
            {
                std::cout << "Bond torsion " << static_cast<BondTorsion *>(param)->short_desc() << " of residue " << res->desc() << " already added! Skipping..." << std::endl;
				continue;
            }
			
			valid_bondT.push_back(static_cast<BondTorsion *>(param));
			angNames.push_back(static_cast<BondTorsion *> (param)->short_desc());
			std::cout << "Bond torsion " << static_cast<BondTorsion *>(param)->short_desc() << " of residue " << res->desc() << " added!" << std::endl;
		}

		tors_res.push_back(new TorsRes4NN);
		tors_res[i]->torsName.resize(valid_bondT.size());
		tors_res[i]->desc.resize(valid_bondT.size());
		tors_res[i]->bondSymmetry.resize(valid_bondT.size());
        tors_res[i]->v.resize(valid_bondT.size());
		tors_res[i]->nAng = valid_bondT.size();

		for(int j = 0; j < valid_bondT.size(); j++)
		{
			tors_res[i]->ang.push_back(std::vector<double>(numPaths, 0));
			tors_res[i]->torsName[j] = valid_bondT[j]->short_desc();
			tors_res[i]->desc[j] = valid_bondT[j]->desc();
            tors_res[i]->v[j] = valid_bondT[j]->bondMidPoint();

			if((tors_res[i]->desc[j] == "chi2" && (res->code() == "ASP"|| res->code() == "PHE" || res->code() == "TYR")) ||
					(tors_res[i]->desc[j] == "chi3" && res->code() == "GLU") ||
					(tors_res[i]->desc[j] == "chi4" && res->code() == "ARG"))
			{
				tors_res[i]->bondSymmetry[j] = 180.0;
			}
			else
			{
				tors_res[i]->bondSymmetry[j] = 360.0;
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
		pr->submitJobAndRetrieve(0.1, true);

		//Floats ref_coords = Cluster::weights(paths[i].front()->startInstance());

		AtomGroup *content = pr->instance()->currentAtoms();
		//content->recalculate();

		for (int j = 0; j < tors_res.size(); j++)
		{
			for (int k = 0; k < tors_res[j]->nAng; k++)
			{
				Parameter *param = content->findParameter(tors_res[j]->desc[k], polySeq->residue(j)->id());

				tors_res[j]->ang[k][i] = param->empiricalMeasurement();
			}
		}
	}

	return tors_res;
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

struct Entropy* PathEntropy::calculate_entropy_independent(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> tors_res){
	std::vector<std::vector<double>> phit(nf);
	double *a, *sd, *sd_k, *ent_k, *ent_k_2, *ent_k_tot, *ent_k_tot_2;
	int n_res_per_model = tors_res.size();

	int n_tors = 0;

	struct Entropy* entropy = new Entropy;

    allocVariables(nf, &ent_k, &ent_k_tot, &ent_k_2, &ent_k_tot_2, &sd_k, &flagParameters);

	entropy->n_single = n_res_per_model;
	entropy->n_nn = flagParameters.n;
	alloc_entropy(entropy, n_res_per_model, 0, entropy->n_nn, flagParameters);

    int K = flagParameters.n + 1;

	for(int k = 1; k <= K-1; k++)
	{
		entropy->total[k-1] = 0.0;
		entropy->sd_total[k-1] = 0.0;
		entropy->dm_total[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	for(int m = 0; m < n_res_per_model; m++)
		if (tors_res[m]->nAng > 0)
		{
			for(int i = 0; i < nf; i++)
			{
				phit[i] = std::vector<double> (tors_res[m]->nAng);

				for (int j = 0; j < tors_res[m]->nAng; j++)
				{
					phit[i][j] = tors_res[m]->ang[j][i];
					n_tors++;
				}
			}
            
            double d_mean[K];
            double ld_mean[K];            

			for(int i = 0; i < K; i++)
            {
				d_mean[i] = 0;
                ld_mean[i] = 0;
			}

			for(int i = 0; i < K-1; i++)
			{
				ent_k[i] = 0;
				ent_k_2[i] = 0;
			}

			/* calculate the distance between samples in the nAng - dimensional
			   space of torsion angles */

			double d[nf];

			/*auto display = [nf, m](double *d)
			{
			    if (m != 4) { return; }
			    std::cout << "4th residue: ";
			 
                for (int i = 0; i < nf; i++)
			    {
			        std::cout << d[i] << ", ";
			    }
			    std::cout << std::endl;
			};*/


			for(int i = 0; i < nf; i++)
			{
               
				for(int j = 0; j < nf; j++)
				{
					d[j] = dist_ang(phit[i], phit[j], tors_res[m]->nAng, tors_res[m]->bondSymmetry);
					d[j] = deg2rad(d[j]);
				}

                //display(d);

				/* sort the distances */
				qsort(d, nf, sizeof(double), comp);

				/* apply the entropy calculation based on the nearest neighbour */
				for(int k = 1; k < K; k++)
				{
					/* if the distance is less than a pre-set value, reset the 
					 * distance to the pre-set values, to avoid NaNs */

                    double logdk;

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

			for(int k = 1; k < K; k++)
			{
				ent_k[k-1] = ent_k[k-1] * ((double) tors_res[m]->nAng / (double) nf);
				ent_k_2[k-1] = ent_k_2[k-1] * (double) tors_res[m]->nAng * (double) tors_res[m]->nAng / (double) nf;
			}

            double c = 0.0;
            int L = 0;

			for(int k = 0; k < tors_res[m]->nAng; k++)
			{
				c = c - log(tors_res[m]->bondSymmetry[k] * M_PI / 180.0);
            }
				
            c = c + (((double) tors_res[m]->nAng) * log(M_PI)/2.0) - lgamma(1.0 +  ((double) tors_res[m]->nAng)/2.0) + 0.5722 + log((double) nf);

			for(int k = 1; k < K; k++)
			{
				// before adding c-L compute sd
				ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
				sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
				ent_k[k-1] = ent_k[k-1] + c - L;
				L = L + 1/k;

				d_mean[k] = d_mean[k] / (double) nf;
				ld_mean[k] = ld_mean[k] / (double) nf;

				entropy->h1[m][k-1] = ent_k[k-1];
				entropy->dm1[m][k-1] = d_mean[k];
				entropy->sd1[m][k-1] = sd_k[k-1];
				entropy->total[k-1] = entropy->total[k-1] + ent_k[k-1];
				entropy->sd_total[k-1] = entropy->sd_total[k-1] + entropy->sd1[m][k-1] * entropy->sd1[m][k-1];
				entropy->dm_total[k-1] = entropy->dm_total[k-1] + entropy->dm1[m][k-1] * entropy->dm1[m][k-1];
			}

			int weightCheck = 1;

            double x[K-1];
            double y[K-1];
            double w[K-1];

			for(int k = 0; k < K-1; k++)
			{
				x[k] = ent_k[k];
				y[k] = d_mean[k+1];

                entropy->totalEntropy += ent_k[0];

				if(sd_k[k] > 1e-12)
					w[k] = 1/(sd_k[k] * sd_k[k]);
				else
					weightCheck = 0;
			}

			/* if one of the weights for the linear fit is infinite,
			 * set all weights to 1 */
			if(weightCheck == 0)
			{
				for(int k = 0; k < K-1; k++)
					w[k] = 1;
			}

			double a[3];
            double sd[3];

			fitlw(x, y, w, K-1, a, sd, &weightCheck);

			entropy->h1lm[m] = a[0];
			entropy->sd1lm[m] = sd[0];
			//entropy->dm1lm[m] = d_mean[1]/sqrt((double) tors_res[m]->nAng);
			//entropy->totalEntropy = entropy->totalEntropy + a[0];
            entropy->totalEntropy = entropy->totalEntropy/(double) (K-1);
		}

	for(int k = 0; k < flagParameters.n; k++)
	{
		entropy->sd_total[k] = sqrt(entropy->sd_total[k]);
		entropy->dm_total[k] = sqrt(entropy->dm_total[k]/ (double) n_tors);
	}

	return entropy;

}

/* Calculates entropy using mutual information for torsions closer in space than a given value */

struct Entropy* PathEntropy::calculate_entropy_mi(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> tors_res)
{
	int *group2res;
	std::vector<std::vector<double>> phit(nf); 
    double *ent_k, *ent_k_tot, *ent_k_2, *sd_k, *ent_k_tot_2, *x, *y, *w, *d;

	int n_res_per_model = tors_res.size();
    int n_res_per_model_mi = 0;
	std::vector<TorsRes4NN*> tors_mi;

	struct TorsRes4NN tors_mi2;
	struct Entropy *entropy = new Entropy;

    int nTors = 0;

    allocVariables(nf, &ent_k, &ent_k_tot, &ent_k_2, &ent_k_tot_2, &sd_k, &flagParameters);

    entropy->n_nn = flagParameters.n;
    alloc_entropy(entropy, n_res_per_model, 0, entropy->n_nn, flagParameters);

	// for each residue...
	int K = flagParameters.n + 1;
	
    tors_res2mi(tors_res, n_res_per_model, tors_mi, n_res_per_model_mi, group2res, flagParameters);
    
    for(int k = 1; k <= K-1; k++)
	{
		entropy->total[k-1] = 0.0;
		entropy->sd_total[k-1] = 0.0;
		entropy->dm_total[k-1] = 0.0;
	}

	//... based on a cutoff distance, calculate how many pairs of groups must be considered
	entropy->n_single = n_res_per_model_mi;
	entropy->n_pair = 0;

	for(int i = 0; i < n_res_per_model_mi; i++)
		for(int j = i+1; j < n_res_per_model_mi; j++)
		{
			for(int l = 0; l < tors_mi[i]->nAng; l++)
				for(int m = 0; m < tors_mi[j]->nAng; m++)
				{
                    glm::vec3 u(tors_mi[i]->v[l]);
                    glm::vec3 v(tors_mi[j]->v[m]);

					if(glm::distance(u, v) <= flagParameters.cutoff)
					{
						l = tors_mi[i]->nAng + 1;
						m = tors_mi[j]->nAng + 1;
						entropy->n_pair++;
					}
				}
		}

	// for each group, compute entropy, sd and dm for the group and map to residues
	// sum to total entrop, sd and dm
	// normalise dm by sqrt(dof)
	for(int m = 0; m < n_res_per_model_mi; m++)
	{
		for(int i = 0; i < nf; i++)
		{
			phit[i] = std::vector<double> (tors_mi[m]->nAng);

			for(int j = 0; j < tors_mi[m]->nAng; j++)
			{
				phit[i][j] = tors_mi[m]->ang[j][i];
                nTors++;
			}
		}

        double d_mean[K];
        double ld_mean[K];

		for(int i = 0; i < K-1; i++)
		{
			ent_k_2[i] = 0;
            ent_k[i] = 0;
		}

		for(int i = 0; i < K; i++)
		{
			d_mean[i] = 0;
            ld_mean[i] = 0;
		}

        double d[nf];

		for(int i = 0; i < nf; i++)
		{
			for(int j = 0; j < nf; j++)
			{
				d[j] = dist_ang(phit[i], phit[j], tors_mi[m]->nAng, tors_mi[m]->bondSymmetry);
				d[j] = deg2rad(d[j]);
			}
		}

		qsort(d,nf,sizeof(double), comp);

		for(int k = 1; k < K; k++)
		{
            double logdk;
          
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

		for(int k = 1; k < K; k++)
		{
			ent_k[k-1] = ent_k[k-1] * ((double) tors_mi[m]->nAng / (double) nf);
			ent_k_2[k-1] = ent_k_2[k-1] * (double) tors_mi[m]->nAng * (double) tors_mi[m]->nAng / (double) nf;
		}
 
        double c = 0.0;

		for(int k = 0; k < tors_mi[m]->nAng; k++)
		{
			c = c - log(M_PI /180.0);
			c = c + (double) tors_mi[m]->nAng * log(M_PI)/2.0 - lgamma(1.0 + (double) tors_mi[m]->nAng)/2.0 + 0.5722 + log((double) nf);
		}

        int L = 0;

		for(int k = 1; k < K; k++)
		{
			// before adding c-L compute sd
			ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
			sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
			// add c-L 
			ent_k[k-1] = ent_k[k-1] + c - L;
			L = L + 1/k;

			d_mean[k] = d_mean[k]/(double) nf;
			ld_mean[k] = ld_mean[k]/(double) nf;

			entropy->h1[m][k-1] = ent_k[k-1];
			entropy->dm1[m][k-1] = d_mean[k];
			entropy->sd1[m][k-1] = sd_k[k-1];
		}

		// Linear weighted fit
		int  ok = 1;

		for(int k = 0; k < K-1; k++)
		{
			x[k] = d_mean[k+1];
			y[k] = ent_k[k];

			if(sd_k[k] > 0) w[k] = 1/(sd_k[k] * sd_k[k]);
			else ok = 0;
		}

		if(ok == 0)
		{
			for(int k = 0; k< K-1; k++)
				w[k] = 1;
		}

	
        double a[3];
        double sd[3];	

        fitlw(x,y,w,K-1,a,sd,&ok);

		entropy->h1lm[m] = a[0];
		entropy->sd1lm[m] = sd[0] * sd[0];
		entropy->dm1lm[m] = d_mean[1] * d_mean[1];
	}

	// ... then prepare for mutual information calculation ... 
	tors_mi2.ang.resize(2*flagParameters.kmi);

	for(int i = 0; i < nf; i++)
	{
		phit[i].resize(2*flagParameters.kmi);
	}

	int kk = 0;
	// ... and calculate the entropy for paired groups of torsions 
	// with the nearest neighbour method ... 
	for(int ii = 0; ii < n_res_per_model_mi; ii++)
		for(int jj = ii + 1; jj < n_res_per_model_mi; jj++)
			for(int l = 0; l < tors_mi[ii]->nAng; l++)
				for(int m = 0; m < tors_mi[jj]->nAng; m++)
					if(glm::distance(tors_mi[ii]->v[l], tors_mi[jj]->v[m]) <= flagParameters.cutoff)
					{
						entropy->i1[kk] = ii;
						entropy->i2[kk] = jj;

						l = tors_mi[ii]->nAng + 1;
						m = tors_mi[jj]->nAng + 1;
						tors_mi2.nAng = tors_mi[ii]->nAng + tors_mi[jj]->nAng;

						int i = 0;
						
						for(int k = 0; k < tors_mi[ii]->nAng; k++)
						{
							tors_mi2.ang[i] = tors_mi[ii]->ang[k];
						}

						for(int k = 0; k < tors_mi[jj]->nAng; k++)
						{
							tors_mi2.ang[i] = tors_mi[jj]->ang[k];
						}

						for(int i = 0; i < nf; i++)
							for(int j = 0; j < tors_mi2.nAng; j++)
								phit[i][j] = tors_mi2.ang[j][i];

						for(int i = 0; i < K-1; i++)
						{
							ent_k_2[i] = ent_k[i] = 0.0;
						}

                        double d_mean[K];
                        double ld_mean[K];

						for(int i = 0; i < K; i++)
						{
							d_mean[i] = 0;
                            ld_mean[i] = 0;
						}

						d = new double[nf];

						for(int j = 0; j < nf; j++)
						{
							d[j] = dist_ang(phit[i], phit[j], tors_mi2.nAng, tors_mi2.bondSymmetry);
							d[j] = deg2rad(d[j]);
						}

						qsort(d, nf, sizeof(double), comp);

						for(int k = 1; k < K; k++)
						{
                            double logdk; 

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

						for(int k = 1; k<K; k++)
						{
							ent_k[k-1] = ent_k[k-1] * ((double) tors_mi2.nAng / (double) nf);
							ent_k_2[k-1] = ent_k_2[k-1] * ((double) tors_mi2.nAng * (double) tors_mi2.nAng/ (double) nf);
						}

						double c = 0.0;

						for(int k = 0; k < tors_mi2.nAng; k++)
						{
						    c = c - log(tors_mi2.bondSymmetry[k] * M_PI / 180.0);
                        }
						
						c = c + ((double) tors_mi2.nAng) * log(M_PI)/2.0 - lgamma(1.0 + ((double) tors_mi2.nAng)/2.0) + 0.5722 + log( (double) nf);
					   
						int L = 0;

						for(int k = 1; k <= K - 1; k++)
						{
							// before adding c-L compute sd
							ent_k_tot_2[k-1] = ent_k_tot_2[k-1] + ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1];
							sd_k[k-1] = sqrt(ent_k_2[k-1] - ent_k[k-1]*ent_k[k-1]);
							ent_k[k-1] = ent_k[k-1] + c - L;
							L = L + 1/k;
							d_mean[k] = d_mean[k]/(double) nf;
							ld_mean[k] = ld_mean[k]/(double) nf;
							ent_k_tot[k-1] = ent_k_tot[k-1] + ent_k[k-1];
						}

						//... compute, by subtraction of single group entropies, the mutual information ...
						for(int k = 0; k < K - 1; k++)
						{
							entropy->h2[kk][k] = ent_k[k];
							entropy->sd2[kk][k] = sd_k[k];
							entropy->dm2[kk][k] = d_mean[k];
							entropy->mi[kk][k] = entropy->h2[kk][k] - entropy->h1[entropy->i1[kk]][k] - entropy->h1[entropy->i2[kk]][k];
							entropy->sdmi[kk][k] =  pow(entropy->sd2[kk][k],2) + pow(entropy->sd1[entropy->i1[kk]][k],2) + pow(entropy->sd1[entropy->i2[kk]][k],2);
							entropy->dmmi[kk][k] = pow(entropy->dm2[kk][k],2) + pow(entropy->dm1[entropy->i1[kk]][k],2) + pow((*entropy).dm1[entropy->i2[kk]][k],2);
						}

						// linear weighted fit
						int ok = 1;

						for(int k = 0; k < K - 1; k++)
						{
							x[k] = d_mean[k+1];
							y[k] = ent_k[k];

							if(k == 0) w[k] = M_PI * M_PI /6;
							else w[k] = w[k-1] - 1/(double) (k*k);
						}

						double a[3];
						double sd[3];

						fitlw(y,x,w,K-1,a,sd,&ok);

						entropy->h2lm[kk] = a[0]; 
						entropy->sd2lm[kk] = sd[0]; 
						entropy->dm2lm[kk] = d_mean[1];
						entropy->milm[kk] = entropy->h2lm[kk] - entropy->h1lm[entropy->i1[kk]] - entropy->h1lm[entropy->i2[kk]]; 
						kk++;
					}

	return entropy;

}

void PathEntropy::tors_res2mi(std::vector<TorsRes4NN*> tors_res, int n_res_per_model, std::vector<TorsRes4NN*> &tors_mi, int n_res_per_model_mi, int *group2res, struct FlagParameters flagParameters)
{
	int l = 0;

	for(int i = 0; i < n_res_per_model; i++)
	{
		l = l + (int) floor((double) tors_res[i]->nAng / (double) flagParameters.kmi);

		if((tors_res[i]->nAng % flagParameters.kmi) !=0) l++; 
	}

	n_res_per_model_mi = l;

	group2res = new int[n_res_per_model_mi];

	for(int i = 0; i < n_res_per_model_mi; i++)
	{
        tors_mi.push_back(new TorsRes4NN);

		tors_mi[i]->ang.push_back(std::vector<double>(flagParameters.kmi, 0));
		tors_mi[i]->v.resize(flagParameters.kmi);
		tors_mi[i]->bondSymmetry.resize(flagParameters.kmi);
		tors_mi[i]->torsName.resize(flagParameters.kmi);
	}

	// group torsions
	for(int i = 0, j = 0; i < n_res_per_model; i++)
	{
		for(int k = 0; k < tors_res[i]->nAng; k++)
		{
			tors_mi[j]->bondSymmetry[k % flagParameters.kmi] = tors_res[i]->bondSymmetry[k];
			tors_mi[j]->ang[k % flagParameters.kmi] = tors_res[i]->ang[k];
			tors_mi[j]->v[k % flagParameters.kmi] = tors_res[i]->v[k];
			tors_mi[j]->nAng = k % flagParameters.kmi + 1;

		    if(( k % flagParameters.kmi) == (flagParameters.kmi - 1) || k == (tors_res[i]->nAng - 1))
		    {
			    group2res[j] = i;
			    j++;
		    }
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
int PathEntropy::fitlw(double *x, double *y, double *w, int n, double (&a)[3], double (&sd)[3], int *ok)
{
	double wt = 0;
	double xm = 0;
	double ym = 0;
	double x2 = 0;
	double y2 = 0;
	double xy = 0;
    double xy2 = 0;

	for(int i = 0; i < n; i++)
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

	a[1] = (xy - xm*ym)/(y2 - ym*ym);
	a[0] = xm - a[1]*ym;

	double sig2 = 0;

	for(int i = 0; i < n; i++)
	{
		sig2 = sig2 + (x[i] - a[0] - a[1]*y[i])*(x[i] - a[0] - a[1]*y[i])*w[i];
	}

	sig2 = sig2/wt;

	sd[0] = sqrt(sig2 * (double) n / (double) (n-2)) * sqrt((1.0/(double) n) + ym*ym/((double) n * (y2 - ym*ym)));
	sd[1] = sqrt(sig2 * (double) n / (double) (n-2)) / sqrt((double) n * (y2 - ym*ym));
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

/* allocates memory to entropy calculation variables */
int PathEntropy::allocVariables(int nf, double **ent_k, double **ent_k_tot, double **ent_k_2, double **ent_k_tot_2, double **sd_k, struct FlagParameters *flagParameters)
{
    int K = flagParameters->n + 1;
    
    if (K > nf)
    {
        K = nf;
        flagParameters->n = K - 1;
    }

	*sd_k = new double[K-1];

	*ent_k = new double[K-1]; 
	*ent_k_2 = new double[K-1];
    *ent_k_tot = new double[K-1];
	*ent_k_tot_2 = new double[K-1];
}

