#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
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
	flagParameters.mist = false;
    flagParameters.kmi = 1; /* grouping of torsions within the same residue for mutual information calculations. Mutual information among groups will involve at most 2k torsions */

    return flagParameters;
}

std::vector<TorsRes4NN*> PathEntropy::getAtomsAndResidues(int numPaths, const std::vector<Path*> paths, int numDivisions)
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
        torsRes[i]->ang.resize(numDivisions);

		for(int j = 0; j < validBondT.size(); j++)
		{
            for(int l = 0; l < numDivisions; l++)
            {
			    torsRes[i]->ang[l].push_back(std::vector<double>(numPaths, 0));
            }

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

        for(int l = 0; l < numDivisions; l++)
        {
            float time = (l+1)/((double) numDivisions+1.0);

		//	workingPath->submitJobAndRetrieve(time, true);

			for (int j = 0; j < torsRes.size(); j++)
			{
				for (int k = 0; k < torsRes[j]->nAng; k++)
				{
					Parameter *param = content->findParameter(torsRes[j]->desc[k], polySeq->residue(j)->id());

					torsRes[j]->ang[l][k][i] = param->empiricalMeasurement();
				 
				}

			}
        }
	}

	return torsRes;
}


/* Calculates entropy from torsion angles, assuming independence between the residues */

struct EntropyForMatrix PathEntropy::calculateEntropyIndependent(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> torsRes, int numDivisions)
{
	int numResPerModel = torsRes.size();

	int numTors = 0;

	struct Entropy* entropy = new Entropy;
    struct EntropyForMatrix ent4Matrix;

	entropy->nSingle = numResPerModel;
	entropy->nNearestNeighbours = flagParameters.n;
	allocEntropy(entropy, numResPerModel, 0, entropy->nNearestNeighbours, flagParameters);

    int K = flagParameters.n + 1;

	/* for each residue, compute entropy, sd and dm for the residue
	   sum to total entropy, sd and dm
	   dm will be normalised by sqrt(dof)  */

    for(int n = 0; n < numDivisions; n++)
    {
        kNearestNeighbours(torsRes, entropy, flagParameters, numTors, nf, numResPerModel, K, numDivisions);
 
        ent4Matrix.totalEntropy.push_back(entropy->totalEntropy);
		
        for(int k = 0; k < flagParameters.n; k++)
		{
			entropy->sigmaTotal[k] = sqrt(entropy->sigmaTotal[k]);
			entropy->meanDistTotal[k] = sqrt(entropy->meanDistTotal[k]/ (double) numTors);
		}
    }

	return ent4Matrix;
}

/* Calculates entropy using mutual information for torsions closer in space than a given value */

struct EntropyForMatrix PathEntropy::calculateEntropyMI(int nf, struct FlagParameters flagParameters, std::vector<TorsRes4NN*> torsRes, int numDivisions)
{
	int *group2res;
    int numResPerModelMI;
	int numResPerModel = torsRes.size();
	std::vector<TorsRes4NN*> torsMi;

	struct TorsRes4NN torsMi2;
	struct Entropy *entropy = new Entropy;
    struct EntropyForMatrix ent4Matrix;

    int nTors = 0;

    entropy->nNearestNeighbours = flagParameters.n;
    
    // for each residue...
	int K = flagParameters.n + 1;
	
    torsRes2MI(torsRes, numResPerModel, torsMi, numResPerModelMI, group2res, flagParameters, numDivisions);
    
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

    // for each group, compute entropy, sd and dm for the group and map to residues
	// sum to total entrop, sd and dm
	// normalise dm by sqrt(dof)

    for(int n = 0; n < numDivisions; n++) 
    {
         kNearestNeighbours(torsMi, entropy, flagParameters, nTors, nf, numResPerModelMI, K, numDivisions);
    } 

	// ... then prepare for mutual information calculation ... 
    std::vector<std::vector<double>> phit(nf); 
    std::vector<double> entk, entkTotal, entk2, entkTotal2, sigmak;

	for(int i = 0; i < nf; i++)
	{
	    phit[i] = std::vector<double> (2*flagParameters.kmi);
	}
    
    torsMi2.ang.resize(2*flagParameters.kmi);
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
                            torsMi2.bondSymmetry[i] = torsMi[ii]->bondSymmetry[k];
                            i++;
						}

						for(int k = 0; k < torsMi[jj]->nAng; k++)
						{
							torsMi2.ang[i] = torsMi[jj]->ang[k];
                            torsMi2.bondSymmetry[i] = torsMi[jj]->bondSymmetry[k];
                            i++;
						}

						for(int i = 0; i < nf; i++)
							for(int j = 0; j < torsMi2.nAng; j++)
								phit[i][j] = torsMi2.ang[j][i][0];

						for(int i = 0; i < K-1; i++)
						{
							entk2[i] = entk[i] = 0.0;
						}

                        std::vector<double> meanDist(K, 0.0);
                        std::vector<double> meanLogDist(K, 0.0);
						std::vector<double> d(nf);

                        for(int i = 0; i < nf; i++)
                        {
							for(int j = 0; j < nf; j++)
							{
								d[j] = dist_ang(phit[i], phit[j], torsMi2.nAng, torsMi2.bondSymmetry);
								d[j] = deg2rad(d[j]);
							}

							std::sort(d.begin(), d.end());

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
							entropy->sdmi[kk][k] = pow(entropy->sd2[kk][k],2) + pow(entropy->sd1[entropy->i1[kk]][k],2) + pow(entropy->sd1[entropy->i2[kk]][k],2);
							entropy->dmmi[kk][k] = pow(entropy->dm2[kk][k],2) + pow(entropy->dm1[entropy->i1[kk]][k],2) + pow((*entropy).dm1[entropy->i2[kk]][k],2);
						}

						// linear weighted fit
						int ok = 1;
        
                        std::vector<double> x(K-1);
                        std::vector<double> y(K-1);
                        std::vector<double> w(K-1);

						for(int k = 0; k < K - 1; k++)
						{
							x[k] = meanDist[k+1];
							y[k] = entk[k];

							if(k == 0) w[k] = M_PI * M_PI /6;
							else w[k] = w[k-1] - 1/(double) (k*k);
						}

						std::vector<double> a(2);
						std::vector<double> sd(2);

						fitlw(y,x,w,K-1,a,sd);

						entropy->h2lm[kk] = a[0]; 
						entropy->sd2lm[kk] = sd[0]; 
						entropy->dm2lm[kk] = meanDist[1];
						entropy->milm[kk] = entropy->h2lm[kk] - entropy->h1lm[entropy->i1[kk]] - entropy->h1lm[entropy->i2[kk]];
                    	kk++;
					}

    kruskal(entropy, group2res, flagParameters);

    entropy->sigmaTotalEntropy = sqrt(entropy->sigmaTotalEntropy);
    
	return ent4Matrix;

}

int PathEntropy::torsRes2MI(std::vector<TorsRes4NN*> torsRes, int numResPerModel, std::vector<TorsRes4NN*> &torsMi, int &numResPerModelMI, int *group2res, struct FlagParameters flagParameters, int numDivisions)
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
        
        for(int j = 0; j < numDivisions; j++)
        {
		    torsMi[i]->ang[j].push_back(std::vector<double>(flagParameters.kmi, 0));
        }

		torsMi[i]->v.resize(flagParameters.kmi);
		torsMi[i]->bondSymmetry.resize(flagParameters.kmi);
	}

	// group torsions

    int j = 0;

	for(int i = 0; i < numResPerModel; i++)
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

void PathEntropy::kNearestNeighbours(std::vector<TorsRes4NN*> torsRes, struct Entropy* entropy, struct FlagParameters flagParameters, int &numTors, int nf, int numResPerModel, int K, int timeDivisions)
{ 
    std::vector<std::vector<double>> phit(nf); 
    std::vector<double> entk, entkTotal, entk2, entkTotal2, sigmak;

    allocVariables(nf, entk, entkTotal, entk2, entkTotal2, sigmak, flagParameters);
   
    std::ofstream outputLR("linear_regression.csv");
 
    for(int m = 0; m < numResPerModel; m++)
	{	
        if (torsRes[m]->nAng > 0)
			{
				for(int i = 0; i < nf; i++)
				{
					phit[i] = std::vector<double> (torsRes[m]->nAng);

					for (int j = 0; j < torsRes[m]->nAng; j++)
					{
						phit[i][j] = torsRes[m]->ang[0][j][i];
						numTors++;
					}
				}
				
				std::vector<double> meanDist(K);
				std::vector<double> meanLogDist(K);            

				for(int i = 0; i < K-1; i++)
				{
					entk[i] = 0;
					entk2[i] = 0;
				}

				/* calculate the distance between samples in the nAng - dimensional
				   space of torsion angles */

				std::vector<double> d(nf);

				for(int i = 0; i < nf; i++)
				{
				   
					for(int j = 0; j < nf; j++)
					{
						d[j] = dist_ang(phit[i], phit[j], torsRes[m]->nAng, torsRes[m]->bondSymmetry);
						d[j] = deg2rad(d[j]);
					}

					/* sort the distances */
					std::sort(d.begin(), d.end());

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

				std::vector<double> x(K-1);
				std::vector<double> y(K-1);
				std::vector<double> w(K-1);

				for(int k = 0; k < K-1; k++)
				{
					y[k] = entk[k];
					x[k] = meanDist[k+1];

                    outputLR << "entk[" << k << "], " << y[k] << "\t" << "meanDist[" << k+1 << "], " << x[k] << std::endl;
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

				std::vector<double> a(2);
				std::vector<double> sd(2);

				fitlw(y, x, w, K-1,a,sd);

                outputLR << std::endl << "Intercept: " << a[0] << "\t" << "Slope: " << a[1] << "\n\n";

				entropy->h1lm[m] = a[0];
				entropy->sd1lm[m] = sd[0];
				//entropy->dm1lm[m] = meanDist[1]/sqrt((double) torsRes[m]->nAng);
				entropy->totalEntropy = entropy->totalEntropy + a[0];
	    }
    }          

    outputLR.close();
}

/* linear weighting function */
int PathEntropy::fitlw(std::vector<double> y, std::vector<double> x, std::vector<double> w, int n, std::vector<double> &a, std::vector<double> &sd)
{
	double wt = 0.0;
	double xm = 0.0;
	double ym = 0.0;
	double x2 = 0.0;
	double y2 = 0.0;
	double xy = 0.0;
    double xy2 = 0.0;

	for(int i = 0; i < n; i++)
	{
		xm += x[i]*w[i];
		ym += y[i]*w[i];
		x2 += x[i]*x[i]*w[i];
		y2 += y[i]*y[i]*w[i];
		xy += x[i]*y[i]*w[i];
		xy2 += x[i]*y[i]*x[i]*y[i]*w[i];
		wt += w[i];
	}

	xm = xm/wt;
	ym = ym/wt;
	x2 = x2/wt;
	y2 = y2/wt;
	xy = xy/wt;
	xm = xm/wt;

	a[1] = (xy - xm*ym)/(x2 - xm*xm);
	a[0] = ym - a[1]*xm;

	double sig2 = 0;

	for(int i = 0; i < n; i++)
	{
		sig2 += (y[i] - a[0] - a[1]*x[i])*(y[i] - a[0] - a[1]*x[i])*w[i];
	}

	sig2 = sig2/wt;

	sd[0] = sqrt(sig2 * (double) n / (double) (n-2)) * sqrt((1.0/(double) n) + xm*xm/((double) n * (x2 - xm*xm)));
	sd[1] = sqrt(sig2 * (double) n / (double) (n-2)) / sqrt((double) n * (x2 - xm*xm));
}

void PathEntropy::kruskal(struct Entropy *entropy, int *group2res, struct FlagParameters flagParameters)
{
    std::vector<Edge*> edges;
    std::vector<Edge*> MST;

    int set[entropy->nSingle];

    for(int i = 0; i < entropy->nPairs; i++)
    {
        edges.push_back(new Edge);
        edges[i]->u = entropy->i1[i];
        edges[i]->v = entropy->i2[i];
        edges[i]->weight = entropy->milm[i];
        edges[i]->orig = i;
    }

    for(int i = 0; i < flagParameters.n; i++)
    {
        entropy->totalEntropy = 0.0;
        entropy->sigmaTotalEntropy = 0.0;
        entropy->meanDistTotalEntropy = 0.0;
    }

    for(int i = 0; i < entropy->nSingle; i++)
    {
        for(int j = 0; j < flagParameters.n; j++)
        {
            entropy->totalEntropy += entropy->h1[i][j];
            entropy->sigmaTotalEntropy += pow(entropy->sd1[i][j],2.0);
            entropy->meanDistTotalEntropy += pow(entropy->dm1[i][j],2.0);
        }
    }

    for(int i = 0; i < entropy->nSingle; i++)
    {
        set[i]=i;
    }

    std::sort(edges.begin(), edges.end(), [](const Edge* a, const Edge* b)
    {
        return a->weight < b ->weight;
    });

    int counts = 0;

    for(int i = 0; i < entropy->nPairs; i++)
    {
        int j = edges[i]->u;
        int k = edges[i]->v;

        if(set[j] != set[k])
        {
            entropy->mst1[counts] = edges[i]->u;
            entropy->mst2[counts] = edges[i]->v;
            entropy->mstw[counts] = edges[i]->weight;
     
            MST.push_back(new Edge);
            MST[counts]->u = edges[i]->u;
            MST[counts]->v = edges[i]->v;
            MST[counts]->weight = edges[i]->weight;
            MST[counts]->orig = edges[i]->orig;

            counts++;

            int r = set[k];

            for(int l = 0; l < entropy->nSingle; l++)
            {
                if(set[l] == r) set[l] = set[j];
            }
        }
    }
    
    entropy->nEdges = counts;

    if(entropy->nPairs > 0)
    {
        for(int i = 0; i < entropy->nEdges; i++)
        {
            std::cout << "MST weight: " << MST[i]->weight << std::endl;
            for (int kk = 0; kk < flagParameters.n; kk++)
            {
			    entropy->totalEntropy += entropy->h2[MST[i]->orig][kk] - entropy->h1[MST[i]->u][kk] - entropy->h1[MST[i]->v][kk];
			    entropy->sigmaTotalEntropy += pow(entropy->sd2[MST[i]->orig][kk], 2) + pow(entropy->sd1[MST[i]->u][kk], 2) - pow(entropy->sd1[MST[i]->v][kk], 2);
			    entropy->meanDistTotalEntropy += (entropy->dm2[MST[i]->orig][kk] * entropy->dm2[MST[i]->orig][kk]);   
            }
        }
    }

    entropy->totalEntropy = 0.0;

    if(entropy->nPairs > 0)
    {
        for(int i = 0; i < entropy->nEdges; i++)
        {
            entropy->totalEntropy += MST[i]->weight;
        }
    }

    for(int i =0; i < entropy->nSingle; i++)
    {
        entropy->totalEntropy += entropy->h1lm[i];
    }
}

/* allocates memory to entropy structure */
int PathEntropy::allocEntropy(struct Entropy *entropy, int nSingle, int nPairs, int nNearestNeighbours, struct FlagParameters flagParameters)
{
	entropy->pathTotal.resize(nNearestNeighbours);
	entropy->sigmaTotal.resize(nNearestNeighbours);
	entropy->meanDistTotal.resize(nNearestNeighbours);
	entropy->h1lm.resize(nSingle);
	entropy->sd1lm.resize(nSingle);
	entropy->h1.resize(nSingle);
	entropy->sd1.resize(nSingle);
	entropy->dm1.resize(nSingle);

	for(int i = 0; i < nSingle; i++)
	{
		entropy->h1[i].resize(nNearestNeighbours);
		entropy->sd1[i].resize(nNearestNeighbours);
		entropy->dm1[i].resize(nNearestNeighbours);
	}

	if(flagParameters.mutualInformation)
	{
		entropy->mi.resize(entropy->nPairs);
        entropy->mst1.resize(entropy->nPairs);
        entropy->mst2.resize(entropy->nPairs);
        entropy->mstw.resize(entropy->nPairs);
		entropy->i1.resize(entropy->nPairs);
        entropy->i2.resize(entropy->nPairs);
        entropy->h2.resize(entropy->nPairs);
        entropy->sd2.resize(entropy->nPairs);
        entropy->dm2.resize(entropy->nPairs);
        entropy->sdmi.resize(entropy->nPairs);
        entropy->dmmi.resize(entropy->nPairs);
        entropy->h2lm.resize(entropy->nPairs);
        entropy->milm.resize(entropy->nPairs);
        entropy->sd2lm.resize(entropy->nPairs);
        entropy->dm2lm.resize(entropy->nPairs);
    
        for(int i = 0; i < entropy->nPairs; i++)
        {
            entropy->mi[i].resize(nNearestNeighbours);
            entropy->h2[i].resize(nNearestNeighbours);
            entropy->sd2[i].resize(nNearestNeighbours);
            entropy->dm2[i].resize(nNearestNeighbours);
            entropy->sdmi[i].resize(nNearestNeighbours);
            entropy->dmmi[i].resize(nNearestNeighbours);
        }
	}

}

/* allocates memory to entropy calculation variables */
int PathEntropy::allocVariables(int nf, std::vector<double> &entk, std::vector<double> &entkTotal, std::vector<double> &entk2, std::vector<double> &entkTotal2, std::vector<double> &sigmak, struct FlagParameters &flagParameters)
{
    int K = flagParameters.n + 1;
    
    if (K > nf)
    {
        K = nf;
        flagParameters.n = K - 1;
    }

	entk.resize(K-1);
    entkTotal.resize(K-1);
    entk2.resize(K-1);
    entkTotal2.resize(K-1);
    sigmak.resize(K-1);
}

