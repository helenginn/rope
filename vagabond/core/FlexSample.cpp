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


void FlexSample::saveSampledStructures(int numSamples, const std::string& baseFileName, const std::string& csvDistFile, float petrubationWeight)
{
    const Eigen::MatrixXf& V = _flex->getV();
    const Eigen::MatrixXf& S = _flex->getS(); 
    int rangeOfJac = getMatrixRange();

    int totalModes = std::max(0, (int)V.cols());
    int N = V.cols();
    std::vector<int> candidateIndices = sampleColumnIndices(N, numSamples);

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

    int saved = 0;
    int attempts = 0;
    int maxAttempts = numSamples * 50; // safeguard: don't loop forever

    // Reset to base position
    _flex->submitJob(0);
    Result *rInitial = _flex->getResult();
    rInitial->transplantPositions(false, true);

    // backwards iteration
    while (saved < numSamples && attempts < maxAttempts)
    {
        ++attempts;
        // ---------------------------------------------------------
        // DEBUGGING: Force sampling from the last columns of Matrix V
        // ---------------------------------------------------------
        // Instead of using 'candidateIndices', we force the index.
        // Sample 0 -> Last Column (N-1)
        // Sample 1 -> Second to Last (N-2)

        // restore to base position at start of every iteration
        _flex->submitJob(0);
        Result *rReset = _flex->getResult();
        rReset->transplantPositions(false, true);
        rReset->destroy();

        // iterate from floppiest (N-1) toward rigid
        int pickIdx = N - 1 - saved;
        if (pickIdx < rangeOfJac)
        {
            std::cerr << "[FlexSample] Ran out of null space modes.\n";
            break;
        }

        _flex->setColIdx(pickIdx);
        _flex->submitJob(petrubationWeight);
        Result *r = _flex->getResult();
        r->transplantPositions(false);

        float tol = 0.25f;
        bool clashOK = _flex->checkClashes(orderedAtoms, saved, radii, exclude, tol);
        if (!clashOK)
        {
            std::cerr << "[FlexSample] Sample rejected (clash) at attempt " << attempts
                      << " (mode=" << pickIdx << ", weight=" << petrubationWeight << ")\n";
            r->destroy();
            continue; // skip writing this structure
        }

        // save devitations
        if (saveDevOK)
        {
            measureDeviations(saved, pickIdx, petrubationWeight, collectedData);
        }

        std::ostringstream oss; 
        oss << baseFileName << "_mode_" << pickIdx << "_weight" << petrubationWeight << ".pdb"; 
        
        _instance->currentAtoms()->writeToFile(oss.str()); 
        std::cout << "[FlexSample] Saved " << oss.str() 
                  << " (Mode: " << pickIdx << ", Weight: " << petrubationWeight << ")\n";

        r->destroy(); 
        ++saved;
    }
    if (saveDevOK)
    {
        saveBondDeviations(collectedData, csvDistFile);
    }

    if (saved < numSamples) 
    { 
        std::cerr << "[FlexSample] Warning: Could not generate all samples. "
                  << "Saved " << saved << "/" << numSamples 
                  << " (Max attempts reached).\n"; 
    }

}

void FlexSample::measureDeviations(int sampleIdx,  int modeIdx, float weight,
                                    std::vector<DeviationData>& collectedData)
{
    const auto& hbonds = _flex->getHBonds();
    auto cosDHA = [](const glm::vec3& D, const glm::vec3& H, const glm::vec3& A)
    {
        return glm::dot(glm::normalize(D - H), glm::normalize(A - H));
    };

    auto cosHAAA = [](const glm::vec3& H, const glm::vec3& A, const glm::vec3& AA)
    {
        return glm::dot(glm::normalize(H - A), glm::normalize(AA - A));
    };

    auto torsionAngle = [](const glm::vec3& p1, const glm::vec3& p2,
                           const glm::vec3& p3, const glm::vec3& p4)
    {
        glm::vec3 b1 = p2 - p1;
        glm::vec3 b2 = p3 - p2;
        glm::vec3 b3 = p4 - p3;
        glm::vec3 n1 = glm::cross(b1, b2);
        glm::vec3 n2 = glm::cross(b2, b3);
        float b2_norm = glm::length(b2);
        glm::vec3 b2_hat = b2 / b2_norm;
        float x = glm::dot(n1, n2);
        float y = glm::dot(glm::cross(n1, b2_hat), n2);
        return std::atan2(y, x);
    };

    for (size_t bIdx = 0; bIdx < hbonds.size(); ++bIdx)
    {
        const auto& bond = hbonds[bIdx];

        // Constraint 1: H-A distance
        float finalDist = glm::distance(bond.Hydrogen->derivedPosition(),
                                        bond.Acceptor->derivedPosition());
        collectedData.push_back({sampleIdx, modeIdx, weight, (int)bIdx,
            "distance", bond.startDist, finalDist, finalDist - bond.startDist});

        // Constraint 2: cos(D-H-A)
        float finalAlpha = cosDHA(bond.Donor->derivedPosition(),
                                   bond.Hydrogen->derivedPosition(),
                                   bond.Acceptor->derivedPosition());
        collectedData.push_back({sampleIdx, modeIdx, weight, (int)bIdx,
            "angle_DHA", bond.AlphaAngleDist, finalAlpha,
            finalAlpha - bond.AlphaAngleDist});

        // Constraint 3: cos(H-A-AA)
        float finalBeta = cosHAAA(bond.Hydrogen->derivedPosition(),
                                   bond.Acceptor->derivedPosition(),
                                   bond.ParentAcceptor->derivedPosition());
        collectedData.push_back({sampleIdx, modeIdx, weight, (int)bIdx,
            "angle_HAA", bond.BetaAngleDist, finalBeta,
            finalBeta - bond.BetaAngleDist});

        // Constraint 4: dihedral C-D-H-A
        float finalTau1 = torsionAngle(
            bond.ParentDonor->derivedPosition(),
            bond.Donor->derivedPosition(),
            bond.Hydrogen->derivedPosition(),
            bond.Acceptor->derivedPosition());
        collectedData.push_back({sampleIdx, modeIdx, weight, (int)bIdx,
            "dihedral_CDHA", bond.Dihedral1, finalTau1,
            finalTau1 - bond.Dihedral1});

        // Constraint 5: dihedral D-H-A-AA
        float finalTau2 = torsionAngle(
            bond.Donor->derivedPosition(),
            bond.Hydrogen->derivedPosition(),
            bond.Acceptor->derivedPosition(),
            bond.ParentAcceptor->derivedPosition());
        collectedData.push_back({sampleIdx, modeIdx, weight, (int)bIdx,
            "dihedral_DHAA", bond.Dihedral2, finalTau2,
            finalTau2 - bond.Dihedral2});
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
    // print atombyname here
    std::cout << "[debug] In [computeOneSample], inside while, pickIdx (should be equal to _coldIdx) = " << pickIdx << std::endl;
    // * Call submitJob for the petrubationWeight
    _flex->submitJob(static_cast<float>(weight));
    Result *r = _flex->getResult();
    r->transplantPositions(false); // Or true, depending on what you want saved
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
    csv << "SampleIdx,ModeIdx,Weight,BondIdx,ConstraintType,"
        << "StartVal,FinalVal,Diff\n";
    for (const auto& row : data)
    {
        csv << row.sampleIdx    << ","
            << row.modeIdx      << ","
            << row.weight       << ","
            << row.bondIdx      << ","
            << row.constraintType << ","
            << row.startVal     << ","
            << row.finalVal     << ","
            << row.deviation    << "\n";
    }

    std::cout << "[FlexSample] Saved " << data.size() << " deviation records to " << filename << std::endl;
}

int FlexSample::getMatrixRange()
{
    const Eigen::MatrixXf& S = _flex->getS();
    float sigmaCut = 1e-6f;
    int range = 0;
    for (int i = 0; i < S.size(); ++i)
    {
        if (S(i) < sigmaCut) break;
        range++;
    }
    int nullSpaceDim = S.size() - range;

    std::cout << "[DEBUG] Rank (range dimension) = " << range << std::endl;
    std::cout << "[DEBUG] Null space dimension    = " << nullSpaceDim << std::endl;
    std::cout << "[DEBUG] Total cycle DoFs        = " << S.size() << std::endl;

    return range;
}



