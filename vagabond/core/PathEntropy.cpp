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

std::vector<TorsRes4NN*> PathEntropy::getAtomsAndResidues(int numPaths, const std::vector<Path*> paths)
{
	std::vector<TorsRes4NN*> torsRes;
    Sequence *polySeq = static_cast<Polymer *>(paths.front()->startInstance())->sequence();
	
    AtomGroup *rawAtoms = polySeq->convertToAtoms();
		
	for (int i = 0; i < polySeq->size(); i++)
	{
		if(polySeq->residue(i) == nullptr)
		{
			break;
		}

        Residue *res = polySeq->residue(i);

		std::set<TorsionRef> torsions = res->torsions();

		std::vector<BondTorsion*> validBondT;
        std::vector<std::string> angNames;

		for (auto it = torsions.begin(); it != torsions.end(); it++)
		{
			Parameter *param = rawAtoms->findParameter(it->desc(), res->id());

			if (param == nullptr || !param->isTorsion() || param->hasHydrogen())
			{
                continue;
			}
            if (angNames.size() > 0 && std::find(angNames.begin(), angNames.end(), static_cast<BondTorsion *>(param)->shortDesc()) != angNames.end())
            {
				continue;
            }
			
			validBondT.push_back(static_cast<BondTorsion *>(param));
			angNames.push_back(static_cast<BondTorsion *> (param)->shortDesc());
		}

		torsRes.push_back(new TorsRes4NN);
		torsRes[i]->torsName.resize(validBondT.size());
		torsRes[i]->desc.resize(validBondT.size());
		torsRes[i]->bondSymmetry.resize(validBondT.size());
        torsRes[i]->v.resize(validBondT.size());
		torsRes[i]->nAng = validBondT.size();

		for(int j = 0; j < validBondT.size(); j++)
		{
			torsRes[i]->ang.push_back(std::vector<double>(numPaths, 0));
			torsRes[i]->torsName[j] = validBondT[j]->shortDesc();
			torsRes[i]->desc[j] = validBondT[j]->desc();
            torsRes[i]->v[j] = validBondT[j]->bondMidPoint();

			if((torsRes[i]->desc[j] == "chi2" && (res->code() == "ASP"|| res->code() == "PHE" || res->code() == "TYR")) ||
					(torsRes[i]->desc[j] == "chi3" && res->code() == "GLU") ||
					(torsRes[i]->desc[j] == "chi4" && res->code() == "ARG"))
			{
				torsRes[i]->bondSymmetry[j] = 180.0;
			}
			else
			{
				torsRes[i]->bondSymmetry[j] = 360.0;
			}
		}

	}

    PlausibleRoute *workingPath = paths.front()->toRoute();
    workingPath->setup();
    AtomGroup *content = workingPath->all_atoms();

	for (int i = 0; i < numPaths && i < paths.size(); i++)
	{

		if (!paths[i]->startInstance()->hasSequence())
		{
			continue;
		}

		Route *newRoute = paths[i]->toRoute();
        workingPath->transplantFromOtherRoute(newRoute);
        workingPath->submitJobAndRetrieve(0.5, true);

		for (int j = 0; j < torsRes.size(); j++)
		{
			for (int k = 0; k < torsRes[j]->nAng; k++)
			{
				Parameter *param = content->findParameter(torsRes[j]->desc[k], polySeq->residue(j)->id());

                /*if (param == nullptr)
                {
                    std::ofstream ofs;
                    ofs.open("null_params.txt", std::ofstream::out | std::ofstream::app);
                    ofs << torsRes[j]->desc[k] << "\t" << polySeq->residue(j)->id() << "\n";
                    ofs.close();
                }
                else
                {*/
			    torsRes[j]->ang[k][i] = param->empiricalMeasurement();
             
			}
		}
	}

	return torsRes;
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

struct Entropy* PathEntropy::calculateEntropyIndependent(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> torsRes){
	std::vector<std::vector<double>> phit(nf);
	double *a, *sd, *sigmak, *entk, *entk2, *entkTotal, *entkTotal2;
	int numResPerModel = torsRes.size();

	int numTors = 0;

	struct Entropy* entropy = new Entropy;

    allocVariables(nf, &entk, &entkTotal, &entk2, &entkTotal2, &sigmak, &flagParameters);

	entropy->nSingle = numResPerModel;
	entropy->nNearestNeighbours = flagParameters.n;
	allocEntropy(entropy, numResPerModel, 0, entropy->nNearestNeighbours, flagParameters);

    int K = flagParameters.n + 1;

	for(int k = 1; k <= K-1; k++)
	{
		entropy->pathTotal[k-1] = 0.0;
		entropy->sigmaTotal[k-1] = 0.0;
		entropy->meanDistTotal[k-1] = 0.0;
	}

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

	for(int m = 0; m < numResPerModel; m++)
		if (torsRes[m]->nAng > 0)
		{
			for(int i = 0; i < nf; i++)
			{
				phit[i] = std::vector<double> (torsRes[m]->nAng);

				for (int j = 0; j < torsRes[m]->nAng; j++)
				{
					phit[i][j] = torsRes[m]->ang[j][i];
					numTors++;
				}
			}
            
            double meanDist[K];
            double meanLogDist[K];            

			for(int i = 0; i < K; i++)
            {
				meanDist[i] = 0;
                meanLogDist[i] = 0;
			}

			for(int i = 0; i < K-1; i++)
			{
				entk[i] = 0;
				entk2[i] = 0;
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
					d[j] = dist_ang(phit[i], phit[j], torsRes[m]->nAng, torsRes[m]->bondSymmetry);
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

					entk2[k-1] = entk2[k-1] + logdk*logdk;
					entk[k-1] = entk[k-1] + logdk;
					meanDist[k] = meanDist[k] + d[k];
					meanLogDist[k] = meanLogDist[k] + logdk;
				}
			}

			for(int k = 1; k < K; k++)
			{
				entk[k-1] = entk[k-1] * ((double) torsRes[m]->nAng / (double) nf);
				entk2[k-1] = entk2[k-1] * (double) torsRes[m]->nAng * (double) torsRes[m]->nAng / (double) nf;
			}

            double c = 0.0;
            int L = 0;

			for(int k = 0; k < torsRes[m]->nAng; k++)
			{
				c = c - log(torsRes[m]->bondSymmetry[k] * M_PI / 180.0);
            }
				
            c = c + (((double) torsRes[m]->nAng) * log(M_PI)/2.0) - lgamma(1.0 +  ((double) torsRes[m]->nAng)/2.0) + 0.5722 + log((double) nf);

			for(int k = 1; k < K; k++)
			{
				// before adding c-L compute sd
				entkTotal2[k-1] = entkTotal2[k-1] + entk2[k-1] - entk[k-1]*entk[k-1];
				sigmak[k-1] = sqrt(entk2[k-1] - entk[k-1]*entk[k-1]);
				entk[k-1] = entk[k-1] + c - L;
				L = L + 1/k;

				meanDist[k] = meanDist[k] / (double) nf;
				meanLogDist[k] = meanLogDist[k] / (double) nf;

				entropy->h1[m][k-1] = entk[k-1];
				entropy->dm1[m][k-1] = meanDist[k];
				entropy->sd1[m][k-1] = sigmak[k-1];
				entropy->pathTotal[k-1] = entropy->pathTotal[k-1] + entk[k-1];
				entropy->sigmaTotal[k-1] = entropy->sigmaTotal[k-1] + entropy->sd1[m][k-1] * entropy->sd1[m][k-1];
				entropy->meanDistTotal[k-1] = entropy->meanDistTotal[k-1] + entropy->dm1[m][k-1] * entropy->dm1[m][k-1];
			}

			int weightCheck = 1;

            double x[K-1];
            double y[K-1];
            double w[K-1];

			for(int k = 0; k < K-1; k++)
			{
				x[k] = entk[k];
				y[k] = meanDist[k+1];

                entropy->totalEntropy += entk[0];

				if(sigmak[k] > 1e-12)
					w[k] = 1/(sigmak[k] * sigmak[k]);
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
			//entropy->dm1lm[m] = meanDist[1]/sqrt((double) torsRes[m]->nAng);
			//entropy->totalEntropy = entropy->totalEntropy + a[0];
            entropy->totalEntropy = entropy->totalEntropy/(double) (K-1);
		}

	for(int k = 0; k < flagParameters.n; k++)
	{
		entropy->sigmaTotal[k] = sqrt(entropy->sigmaTotal[k]);
		entropy->meanDistTotal[k] = sqrt(entropy->meanDistTotal[k]/ (double) numTors);
	}

	return entropy;

}

/* Calculates entropy using mutual information for torsions closer in space than a given value */

struct Entropy* PathEntropy::calculateEntropyMI(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> torsRes)
{
	int *group2res;
	std::vector<std::vector<double>> phit(nf); 
    double *entk, *entkTotal, *entk2, *sigmak, *entkTotal2, *x, *y, *w, *d;

	int numResPerModel = torsRes.size();
    int numResPerModelMI = 0;
	std::vector<TorsRes4NN*> torsMi;

	struct TorsRes4NN torsMi2;
	struct Entropy *entropy = new Entropy;

    int nTors = 0;

    allocVariables(nf, &entk, &entkTotal, &entk2, &entkTotal2, &sigmak, &flagParameters);

    entropy->nNearestNeighbours = flagParameters.n;
    
    // for each residue...
	int K = flagParameters.n + 1;
	
    torsRes2MI(torsRes, numResPerModel, torsMi, numResPerModelMI, group2res, flagParameters);
    
	//... based on a cutoff distance, calculate how many pairs of groups must be considered
	entropy->nSingle = numResPerModelMI;
	entropy->nPairs = 0;

	for(int i = 0; i < numResPerModelMI; i++)
		for(int j = i+1; j < numResPerModelMI; j++)
		{
			for(int l = 0; l < torsMi[i]->nAng; l++)
				for(int m = 0; m < torsMi[j]->nAng; m++)
				{
                    glm::vec3 u(torsMi[i]->v[l]);
                    glm::vec3 v(torsMi[j]->v[m]);

					if(glm::distance(u, v) <= flagParameters.cutoff)
					{
						l = torsMi[i]->nAng + 1;
						m = torsMi[j]->nAng + 1;
						entropy->nPairs++;
					}
				}
		}

    allocEntropy(entropy, numResPerModelMI, entropy->nPairs, entropy->nNearestNeighbours, flagParameters);

    for(int k = 1; k <= K-1; k++)
	{
		entropy->pathTotal[k-1] = 0.0;
		entropy->sigmaTotal[k-1] = 0.0;
		entropy->meanDistTotal[k-1] = 0.0;
	}

    // for each group, compute entropy, sd and dm for the group and map to residues
	// sum to total entrop, sd and dm
	// normalise dm by sqrt(dof)
	for(int m = 0; m < numResPerModelMI; m++)
	{
		for(int i = 0; i < nf; i++)
		{
			phit[i] = std::vector<double> (torsMi[m]->nAng);

			for(int j = 0; j < torsMi[m]->nAng; j++)
			{
				phit[i][j] = torsMi[m]->ang[j][i];
                nTors++;
			}
		}

        double meanDist[K];
        double meanLogDist[K];

		for(int i = 0; i < K-1; i++)
		{
			entk2[i] = 0;
            entk[i] = 0;
		}

		for(int i = 0; i < K; i++)
		{
			meanDist[i] = 0;
            meanLogDist[i] = 0;
		}

        double d[nf];

		for(int i = 0; i < nf; i++)
		{
			for(int j = 0; j < nf; j++)
			{
				d[j] = dist_ang(phit[i], phit[j], torsMi[m]->nAng, torsMi[m]->bondSymmetry);
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

			entk[k-1] = entk[k-1] + logdk ;
			entk2[k-1] = entk2[k-1] + logdk*logdk;
			meanDist[k] = meanDist[k] + d[k];
			meanLogDist[k] = meanLogDist[k] + logdk;	
		}

		for(int k = 1; k < K; k++)
		{
			entk[k-1] = entk[k-1] * ((double) torsMi[m]->nAng / (double) nf);
			entk2[k-1] = entk2[k-1] * (double) torsMi[m]->nAng * (double) torsMi[m]->nAng / (double) nf;
		}
 
        double c = 0.0;

		for(int k = 0; k < torsMi[m]->nAng; k++)
		{
			c = c - log(M_PI /180.0);
			c = c + (double) torsMi[m]->nAng * log(M_PI)/2.0 - lgamma(1.0 + (double) torsMi[m]->nAng)/2.0 + 0.5722 + log((double) nf);
		}

        int L = 0;

		for(int k = 1; k < K; k++)
		{
			// before adding c-L compute sd
			entkTotal2[k-1] = entkTotal2[k-1] + entk2[k-1] - entk[k-1]*entk[k-1];
			sigmak[k-1] = sqrt(entk2[k-1] - entk[k-1]*entk[k-1]);
			// add c-L 
			entk[k-1] = entk[k-1] + c - L;
			L = L + 1/k;

			meanDist[k] = meanDist[k]/(double) nf;
			meanLogDist[k] = meanLogDist[k]/(double) nf;

			entropy->h1[m][k-1] = entk[k-1];
			entropy->dm1[m][k-1] = meanDist[k];
			entropy->sd1[m][k-1] = sigmak[k-1];
		}

		// Linear weighted fit
		int  weightCheck = 1;

        double x[K-1];
        double y[K-1];
        double w[K-1];

		for(int k = 0; k < K-1; k++)
		{
			x[k] = entk[k];
            y[k] = meanDist[k+1];

			if(sigmak[k] > 1e-12) w[k] = 1/(sigmak[k] * sigmak[k]);
			else weightCheck = 0;
		}

		if(weightCheck == 0)
		{
			for(int k = 0; k< K-1; k++)
				w[k] = 1;
		}

	
        double a[3];
        double sd[3];

        fitlw(x,y,w,K-1,a,sd,&weightCheck);

		entropy->h1lm[m] = a[0];
		entropy->sd1lm[m] = sd[0] * sd[0];
		//entropy->dm1lm[m] = meanDist[1] * meanDist[1];
	}

	// ... then prepare for mutual information calculation ... 

	for(int i = 0; i < nf; i++)
	{
	    phit[i] = std::vector<double> (2*flagParameters.kmi);
        torsMi2.ang.push_back(std::vector<double> (2*flagParameters.kmi, 0));
	}

    torsMi2.bondSymmetry.resize(2*flagParameters.kmi);

	int kk = 0;
	// ... and calculate the entropy for paired groups of torsions 
	// with the nearest neighbour method ... 
	for(int ii = 0; ii < numResPerModelMI; ii++)
		for(int jj = ii + 1; jj < numResPerModelMI; jj++)
			for(int l = 0; l < torsMi[ii]->nAng; l++)
				for(int m = 0; m < torsMi[jj]->nAng; m++)
					if(glm::distance(torsMi[ii]->v[l], torsMi[jj]->v[m]) <= flagParameters.cutoff)
					{
						entropy->i1[kk] = ii;
						entropy->i2[kk] = jj;

						l = torsMi[ii]->nAng + 1;
						m = torsMi[jj]->nAng + 1;
						torsMi2.nAng = torsMi[ii]->nAng + torsMi[jj]->nAng;

						int i = 0;
						
						for(int k = 0; k < torsMi[ii]->nAng; k++)
						{
							torsMi2.ang[i] = torsMi[ii]->ang[k];
                            torsMi2.bondSymmetry[i++] = torsMi[ii]->bondSymmetry[k];
						}

						for(int k = 0; k < torsMi[jj]->nAng; k++)
						{
							torsMi2.ang[i] = torsMi[jj]->ang[k];
                            torsMi2.bondSymmetry[i++] = torsMi[jj]->bondSymmetry[k];
						}

						for(int i = 0; i < nf; i++)
							for(int j = 0; j < torsMi2.nAng; j++)
								phit[i][j] = torsMi2.ang[j][i];

						for(int i = 0; i < K-1; i++)
						{
							entk2[i] = entk[i] = 0.0;
						}

                        double meanDist[K];
                        double meanLogDist[K];

						for(int i = 0; i < K; i++)
						{
							meanDist[i] = 0;
                            meanLogDist[i] = 0;
						}

						d = new double[nf];

						for(int j = 0; j < nf; j++)
						{
							d[j] = dist_ang(phit[i], phit[j], torsMi2.nAng, torsMi2.bondSymmetry);
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

							entk2[k-1] = entk2[k-1] + logdk*logdk;
							entk[k-1] = entk[k-1] + logdk;
							meanDist[k] = meanDist[k] + d[k];
							meanLogDist[k] = meanLogDist[k] + logdk;
						}

						for(int k = 1; k<K; k++)
						{
							entk[k-1] = entk[k-1] * ((double) torsMi2.nAng / (double) nf);
							entk2[k-1] = entk2[k-1] * ((double) torsMi2.nAng * (double) torsMi2.nAng/ (double) nf);
						}

						double c = 0.0;

						for(int k = 0; k < torsMi2.nAng; k++)
						{
						    c = c - log(torsMi2.bondSymmetry[k] * M_PI / 180.0);
                        }
						
						c = c + ((double) torsMi2.nAng) * log(M_PI)/2.0 - lgamma(1.0 + ((double) torsMi2.nAng)/2.0) + 0.5722 + log( (double) nf);
					   
						int L = 0;

						for(int k = 1; k <= K - 1; k++)
						{
							// before adding c-L compute sd
							entkTotal2[k-1] = entkTotal2[k-1] + entk2[k-1] - entk[k-1]*entk[k-1];
							sigmak[k-1] = sqrt(entk2[k-1] - entk[k-1]*entk[k-1]);
							entk[k-1] = entk[k-1] + c - L;
							L = L + 1/k;
							meanDist[k] = meanDist[k]/(double) nf;
							meanLogDist[k] = meanLogDist[k]/(double) nf;
							entkTotal[k-1] = entkTotal[k-1] + entk[k-1];
						}

						//... compute, by subtraction of single group entropies, the mutual information ...
						for(int k = 0; k < K - 1; k++)
						{
							entropy->h2[kk][k] = entk[k];
							entropy->sd2[kk][k] = sigmak[k];
							entropy->dm2[kk][k] = meanDist[k];
							entropy->mi[kk][k] = entropy->h2[kk][k] - entropy->h1[entropy->i1[kk]][k] - entropy->h1[entropy->i2[kk]][k];
							entropy->sdmi[kk][k] =  pow(entropy->sd2[kk][k],2) + pow(entropy->sd1[entropy->i1[kk]][k],2) + pow(entropy->sd1[entropy->i2[kk]][k],2);
							entropy->dmmi[kk][k] = pow(entropy->dm2[kk][k],2) + pow(entropy->dm1[entropy->i1[kk]][k],2) + pow((*entropy).dm1[entropy->i2[kk]][k],2);
						}

						// linear weighted fit
						int ok = 1;

						for(int k = 0; k < K - 1; k++)
						{
							x[k] = meanDist[k+1];
							y[k] = entk[k];

							if(k == 0) w[k] = M_PI * M_PI /6;
							else w[k] = w[k-1] - 1/(double) (k*k);
						}

						double a[3];
						double sd[3];

						fitlw(y,x,w,K-1,a,sd,&ok);

						entropy->h2lm[kk] = a[0]; 
						entropy->sd2lm[kk] = sd[0]; 
						entropy->dm2lm[kk] = meanDist[1];
						entropy->milm[kk] = entropy->h2lm[kk] - entropy->h1lm[entropy->i1[kk]] - entropy->h1lm[entropy->i2[kk]]; 
						kk++;
					}

	return entropy;

}

void PathEntropy::torsRes2MI(std::vector<TorsRes4NN*> torsRes, int numResPerModel, std::vector<TorsRes4NN*> &torsMi, int &numResPerModelMI, int *group2res, struct FlagParameters flagParameters)
{
	int l = 0;

	for(int i = 0; i < numResPerModel; i++)
	{
		l = l + (int) floor((double) torsRes[i]->nAng / (double) flagParameters.kmi);

		if((torsRes[i]->nAng % flagParameters.kmi) !=0) l++; 
	}

	numResPerModelMI = l;

	group2res = new int[numResPerModelMI];

	for(int i = 0; i < numResPerModelMI; i++)
	{
        torsMi.push_back(new TorsRes4NN);

		torsMi[i]->ang.push_back(std::vector<double>(flagParameters.kmi, 0));
		torsMi[i]->v.resize(flagParameters.kmi);
		torsMi[i]->bondSymmetry.resize(flagParameters.kmi);
		torsMi[i]->torsName.resize(flagParameters.kmi);
	}

	// group torsions
	for(int i = 0, j = 0; i < numResPerModel; i++)
	{
		for(int k = 0; k < torsRes[i]->nAng; k++)
		{
			torsMi[j]->bondSymmetry[k % flagParameters.kmi] = torsRes[i]->bondSymmetry[k];
			torsMi[j]->ang[k % flagParameters.kmi] = torsRes[i]->ang[k];
			torsMi[j]->v[k % flagParameters.kmi] = torsRes[i]->v[k];
			torsMi[j]->nAng = k % flagParameters.kmi + 1;

		    if(( k % flagParameters.kmi) == (flagParameters.kmi - 1) || k == (torsRes[i]->nAng - 1))
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
int PathEntropy::allocEntropy(struct Entropy *entropy, int nSingle, int nPairs, int nNearestNeighbours, struct FlagParameters flagParameters)
{
	entropy->pathTotal = new double[nNearestNeighbours];
	entropy->sigmaTotal = new double[nNearestNeighbours];
	entropy->meanDistTotal = new double[nNearestNeighbours];
	entropy->h1lm = new double [nSingle];
	entropy->sd1lm = new double[nSingle];
	entropy->h1 = new double *[nSingle];
	entropy->sd1 = new double *[nSingle];
	entropy->dm1 = new double *[nSingle];

	for(int i = 0; i < nSingle; i++)
	{
		entropy->h1[i] = new double[nNearestNeighbours];
		entropy->sd1[i] = new double[nNearestNeighbours];
		entropy->dm1[i] = new double[nNearestNeighbours];
	}

	if(flagParameters.mutualInformation)
	{
		entropy->mi = new double*[entropy->nPairs];
		entropy->i1 = new int[entropy->nPairs];
        entropy->i2 = new int[entropy->nPairs];
	}

}

/* allocates memory to entropy calculation variables */
int PathEntropy::allocVariables(int nf, double **entk, double **entkTotal, double **entk2, double **entkTotal2, double **sigmak, struct FlagParameters *flagParameters)
{
    int K = flagParameters->n + 1;
    
    if (K > nf)
    {
        K = nf;
        flagParameters->n = K - 1;
    }

	*sigmak = new double[K-1];

	*entk = new double[K-1]; 
	*entk2 = new double[K-1];
    *entkTotal = new double[K-1];
	*entkTotal2 = new double[K-1];
}

