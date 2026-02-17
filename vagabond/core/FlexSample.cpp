#include "FlexSample.h"
#include <vagabond/core/Flexibility.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/AtomGroup.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <numeric>
#include <set>
#include <sstream>

#include "AtomGroup.h"

using Eigen::VectorXf;


FlexSample::FlexSample(Flexibility *flex, Instance *instance) 
    : _flex(flex), _instance(instance)
{
    if (_flex == nullptr)
    {
        throw std::runtime_error("FlexSample requires a valid Flexibility pointer");
    }
}


void FlexSample::saveHierarchySamples(int numSamples, const std::string& baseFileName, float stepSize)
{
    if (!std::isfinite(numSamples) || numSamples <= 0) numSamples = 1;

    const Eigen::MatrixXf& V = _flex->getV();
    int N = V.cols();
    if (N == 0) {
        std::cerr << "[FlexSample] No modes available (V has 0 columns). Aborting.\n";
        return;
    }

    const AtomVector &atoms = _instance->currentAtoms()->atomVector();
    OpSet<Atom*> atom_set(atoms);
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    std::vector<float> radii = _flex->makeRadiiVec(orderedAtoms);
    std::set<std::pair<int,int>> exclude = _flex->makeExcList(atom_set);

    // is this nessecary? i don't do it on the random sampling...
    _flex->submitJob(0.0f);
    Result *rInitial = _flex->getResult();
    rInitial->transplantPositions(false, true);

    int saved = 0;
    int currentModeIdx = N - 1; // Start from the floppiest mode

    // 4. The Loop: Continue until we have enough samples or run out of modes
    while (saved < numSamples && currentModeIdx >= 0)
    {
        _flex->submitJob(0.0f);
        Result *rReset = _flex->getResult();
        rReset->transplantPositions(false, true); 
        rReset->destroy();

        int pickIdx = currentModeIdx;
        currentModeIdx--; // Decrement now so next loop takes the next mode

        _flex->setColIdx(pickIdx);
        _flex->submitJob(stepSize);

        Result *r = _flex->getResult();
        r->transplantPositions(false); 

        float tol = 0.25f;
        bool clashOK = _flex->checkClashes(orderedAtoms, saved, radii, exclude, tol);

        if (!clashOK)
        {
            // REJECT: Don't save, just destroy and loop to the next mode
            std::cout << "[FlexSample] Rejected Mode " << pickIdx 
                      << " (Clash detected). Moving to next mode.\n";
            r->destroy();
            continue; 
        }

        std::ostringstream oss; 
        oss << baseFileName << "_" << saved << "_mode_" << pickIdx << ".pdb"; 
        _instance->currentAtoms()->writeToFile(oss.str()); 
        
        std::cout << "[FlexSample] Saved hierarchy sample: " << oss.str() 
                  << " (Mode: " << pickIdx << ", Step: " << stepSize << ")\n";

        r->destroy();
        saved++; // Only increment saved count if valid
    }

    if (saved < numSamples) 
    { 
        std::cerr << "[FlexSample] Warning: Requested " << numSamples << " samples, but only found "
                  << saved << " clash-free modes before running out.\n"; 
    }
}


void FlexSample::saveSampledStructures(int numSamples, const std::string& baseFileName, const std::string& csvDistFile)
{


    if (!isfinite(numSamples) || numSamples <= 0)
    {
        numSamples = 1;
    }
    const Eigen::MatrixXf& V = _flex->getV();
    const Eigen::MatrixXf& S = _flex->getS();
    int totalModes = std::max(0, (int)V.cols());
    if (totalModes == 0)
    {
        std::cerr << "[FlexSample] No modes available (V has 0 columns). Aborting.\n";
        return;
    }
    // std::vector<int> candidateIndices = getSoftestModeIndices(S);
    int N = V.cols();
    if (N == 0)
    {
        std::cerr << "[FlexSample] No modes available (V has 0 columsn). Aborting. " << std::endl;
    }
    std::vector<int> candidateIndices = sampleColumnIndices(N, numSamples);
    
    // Debug output
    std::cout << "[FlexSample] Selected " << candidateIndices.size() 
              << " modes for sampling." << std::endl;


    // in the future, we might want to use sampleColumnIndices which returns
    // random indeces from a canonical distribution    

    // prepare collision detection
    const AtomVector &atoms = _instance->currentAtoms()->atomVector();
    OpSet<Atom*> atom_set(atoms);
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    std::vector<float> radii = _flex->makeRadiiVec(orderedAtoms);

    // We use the exclude list to ignore bonded atoms (they don't clash)
    std::set<std::pair<int,int>> exclude = _flex->makeExcList(atom_set);

    // Prepare for Deviation Calculation
    std::vector<DeviationData> collectedData;
    const auto& hbonds = _flex->getHBonds(); // Requires getter in Flexibility.h


    // Random Number Generator (used for the weights)
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<> dis(-5.0, 5.0); // debug: set a wegith of 1 for now

    int saved = 0;
    int attempts = 0;
    int maxAttempts = numSamples * 50; // safeguard: don't loop forever

    // Reset to base position
    _flex->submitJob(0);
    Result *rInitial = _flex->getResult();
    rInitial->transplantPositions(false, true);
    
    
    // backwards iteration
    int candidateIdx = candidateIndices.size() - 1;
    
    while (saved < numSamples && attempts < maxAttempts)
    {
        ++attempts;
        // ---------------------------------------------------------
        // DEBUGGING: Force sampling from the last columns of Matrix V
        // ---------------------------------------------------------
        // Instead of using 'candidateIndices', we force the index.
        // Sample 0 -> Last Column (N-1)
        // Sample 1 -> Second to Last (N-2)
        // int pickIdx = N - 1 - saved; // previoud
        int pickIdx = saved; // debug

        // If we run out of indices going backwards, reset to the end
        // if (candidateIdx < 0) 
        // {
        //     candidateIdx = candidateIndices.size() - 1;
        // }
        if (pickIdx < 0)
        {
            pickIdx = 0;
        }

        // Pick mode
        // int pickIdx = candidateIndices[candidateIdx]; 
        _flex->setColIdx(pickIdx);

        // float randomWeight = dis(gen); // change for debugging: float randomWeight = 1;
        float randomWeight = 1;
        _flex->submitJob(randomWeight);
        Result *r = _flex->getResult();
        r->transplantPositions(false);

        // NEW: Measure Deviations
        std::cout << "[DEBUG] hbonds.size() = " << hbonds.size() << std::endl;
        for (size_t bIdx = 0; bIdx < hbonds.size(); ++bIdx)
        {

            const auto& bond = hbonds[bIdx];
            std::cout << "[DEBUG] bond.startDist (must be always the same) = " << bond.startDist << std::endl;
            // Get ACTUAL positions from the modified atoms
            float finalDist = glm::distance(bond.Hydrogen->derivedPosition(), 
                                          bond.Acceptor->derivedPosition());
            
            float deviation = finalDist - bond.startDist;
            std::cout << "Column (Constraint ID): " << bIdx 
                      << " | Deviation: " << std::scientific << deviation 
                      << std::endl;
            collectedData.push_back({
                saved,          // Sample Index
                pickIdx,        // Mode Index
                randomWeight,   // Weight
                (int)bIdx,      // Bond Index
                bond.startDist, // Original Distance
                finalDist,      // New Distance
                finalDist - bond.startDist // Deviation
            });
        }
        

        float tol = 0.25f;
        bool clashOK = _flex->checkClashes(orderedAtoms, saved, radii, exclude, tol);
        if (!clashOK)
        {
            std::cerr << "[FlexSample] Sample rejected (clash) at attempt " << attempts
                      << " (mode=" << pickIdx << ", weight=" << randomWeight << ")\n";
            r->destroy(); 
            candidateIdx--;
            continue; // skip writing this structure
        }

        std::ostringstream oss; 
        oss << baseFileName << "_" << saved << "_mode_" << pickIdx << ".pdb"; 
        
        _instance->currentAtoms()->writeToFile(oss.str()); 
        std::cout << "[FlexSample] Saved " << oss.str() 
                  << " (Mode: " << pickIdx << ", Weight: " << randomWeight << ")\n";

        r->destroy(); 
        ++saved;
        // candidateIdx--;
    }
    saveBondDeviations(collectedData, csvDistFile);


    if (saved < numSamples) 
    { 
        std::cerr << "[FlexSample] Warning: Could not generate all samples. "
                  << "Saved " << saved << "/" << numSamples 
                  << " (Max attempts reached).\n"; 
    }

}


std::vector<int> FlexSample::sampleColumnIndices(int N, int sampleCount)
{
    if (N <= 0 || sampleCount <= 0) return {};
    std::cout << "[debug] Flag 1: Hello from sampleColumnIndices! " << std::endl;
    std::vector<int> weights(N); // numbers from 0 to N
    for (int i = 0; i < N; i++)
    {
        weights[i] = i+1;
    }
    std::cout << "[debug] Flag 2: Finished calculating weights " << std::endl;
    std::vector<int> cumulativeWeights(N);
    cumulativeWeights[0] = weights[0];
    std::cout << "[debug] Flag 3: Assigned  weights[0] to cumulativeWeights[0] = " 
                                            << cumulativeWeights[0] << std::endl;
    for (int i = 1; i < N; i++)
    {
        cumulativeWeights[i] = cumulativeWeights[i-1] + weights[i];
    }
    std::cout << "[debug] Flag 4: Finished calculating cumulative sum! " << std::endl;
    std::vector<int> result;
    result.reserve(sampleCount);
    std::cout << "[debug] Flag 5: About to calculate the ranodm index " << std::endl;
    for (int j = 0; j < sampleCount; ++j)
    {    
        int idx = pickIndex(cumulativeWeights);
        result.push_back(idx);
    }
    std::cout << "[debug] Flag 6: Finished! Return result " << std::endl;
    return result; 
}



// std::vector<int> Flexibility::sampleColumnIndicesExp(int N, int sampleCount, double lambda)
// {
//     std::vector<double> weights(N);
//     for (int i=0; i<N; i++)
//     {
//         weights[i] = std::exp(lambda * i);
//     }
//     // normalise weights 
//     double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
//     for (auto& w : weights) w /= sum;

//     // compute CDF
//     std::vector<double> cdf(N);
//     cdf[0] = weights[0];
//     for (int i = 1; i < N; ++i)
//         cdf[i] = cdf[i - 1] + weights[i];
//     // Sample indices
//     std::vector<int> sampled;
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_real_distribution<> dis(0.0, 1.0);
//     while (sampled.size() < sampleCount)
//     {
//         double r = dis(gen);
//         auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
//         int idx = std::distance(cdf.begin(), it);
//         sampled.push_back(idx);
//     }
//     return sampled;
// }

void FlexSample::computeOneSample(int pickIdx, double weight)
{
    _flex->setColIdx(pickIdx);
    _flex->submitJob(static_cast<float>(0));
    {
        Result *r = _flex->getResult(); // FIX: clean up the results in the end 
        r->transplantPositions(false, true); // Or true, depending on what you want saved
        r->destroy();
    }
    // pritn atombyname here
    std::cout << "[debug] In [computeOneSample], inside while, pickIdx (should be equal to _coldIdx) = " << pickIdx << std::endl;
    // * Call submitJob for the randomWeight
    _flex->submitJob(static_cast<float>(weight));
    Result *r = _flex->getResult();
    r->transplantPositions(false); // Or true, depending on what you want saved
    // pritn atombyname here 
    r->destroy(); 
}

// === NEW: MEASURE H-A DISTANCE ===
void FlexSample::saveBondDeviations(const std::vector<DeviationData>& data, const std::string& filename)
{
    std::ofstream csv(filename);
    if (!csv.is_open())
    {
        std::cerr << "[FlexSample] Error: Could not open file" << filename << "for writing.";

    }
     csv <<"SampleIdx, ModeIdx, Weight, BondIdx, StartDist, FinalDist, DistDiff\n";
         for (const auto& row : data)
    {
        csv << row.sampleIdx << "," 
            << row.modeIdx << "," 
            << row.weight << "," 
            << row.bondIdx << "," 
            << row.startDist << "," 
            << row.finalDist << "," 
            << row.deviation << "\n";
    }

    std::cout << "[FlexSample] Saved " << data.size() << " deviation records to " << filename << std::endl;
}



std::vector<int> FlexSample::getSoftestModeIndices(const Eigen::VectorXf& singularValues)
{
    int totalModes = static_cast<int>(singularValues.size());
    std::vector<int> indices;
    for (int i = totalModes -1; i >= 0; i--)
    {
        if (singularValues[i] < 1e-6)
        {
            indices.push_back(i);
        }
    }
    return indices;
}





