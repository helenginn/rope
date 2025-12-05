#include "FlexSample.h"
#include "Flexibility.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

#include "AtomGroup.h"


FlexSample::FlexSample(Flexibility *flex) : _flex(flex)
{
    if (_flex == nullptr)
    {
        throw std::runtime_error("FlexSample requires a valid Flexibility pointer");
    }
}


void FlexSample::saveSampledStructures(int numSamples, const std::string& baseFileName, double lambda)
{
    const Eigen::MatrixXf& _V = _flex->getV();
    const Eigen::MatrixXf& _S = _flex->getV();

    std::cout << "[saveSampledStructures] _S size = " << _S.size() << "\n";
    if (_S.size() > 0)
        std::cout << "First few singular values: " << _S.head(std::min<int>(10, _S.size())).transpose() << "\n";
    else
        std::cerr << "[saveSampledStructures] _S is empty!\n";
    // Call sampleColumnIndices to choose numSamples columns from _V
    // std::vector<int> indices = sampleColumnIndices(_vSize, numSamples, lambda);
    std::vector<int> indices = getSoftestModeIndices(_S);
    // Print the indices and their corresponding singular values

    
    // start - debugging
    const unsigned displayLimit = 5;

    int saved = 0;
    int attempts = 0;
    int maxAttempts = numSamples * 10; // safeguard: don't loop forever

    // prepare radii
    std::set<std::pair<int,int>> exclude;
    std::vector<float> radii;

    const AtomVector &atoms = _flex->_instance->currentAtoms()->atomVector();
    OpSet<Atom*> atom_set(atoms);
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    radii = makeRadiiVec(orderedAtoms);

    // prepare exclude
    exclude = makeExcList(atom_set);

    // * Call submitJob for the randomWeight
    double weight = 0;
    _flex->submitJob(weight);
    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(false, true); // Or true, depending on what you want saved

    while (saved < numSamples && attempts < maxAttempts)
    {
        ++attempts;
        // * store index in _colIdx
        // try:
        // _colIdx = indices[attempts % indices.size()];
        std::cout << "[debug] In [saveSampledStructures], inside while, _colIdx = " << _colIdx << std::endl;
        // _colIdx = indices[saved];

        // * Generate a random scalar weight in [-10, 10] (amplitude of the pertubation along that column)
        std::random_device rd; std::mt19937 gen(rd()); 
        std::uniform_real_distribution<> dis(-5.0, 5.0); 
        double randomWeight = dis(gen);
        // * Call submitJob for the randomWeight
        _flex->submitJob(randomWeight);
        Result *r = _resources.calculator->acquireObject();
        r->transplantPositions(false); // Or true, depending on what you want saved


        // check for classes
        float tol = 0.25f;
        bool clashOK = checkClashes(orderedAtoms, saved, radii, exclude, tol);
        if (!clashOK)
        {
            std::cerr << "[saveSampledStructures] Sample " << saved << " rejected due to atom clash\n"; 
            r->destroy(); 
            continue; // skip writing this structure
        }
        std::ostringstream oss; oss << baseFileName << "_" << saved << ".pdb"; 
        _instance->currentAtoms()->writeToFile(oss.str()); 
        r->destroy(); 
        ++saved;   
    }
    if (saved < numSamples) 
    { 
        std::cerr << "[saveSampledStructures] Warning: only saved " 
        << saved << "/" << numSamples << " due to clashes (maxAttempts=" 
        << maxAttempts << ")\n"; 
    }

}

std::vector<int> FlexSample::sampleColumnIndices(int N, int sampleCount, double lambda)
{
    std::vector<double> weights(N);
    for (int i=0; i<N; i++)
    {
        weights[i] = std::exp(lambda * i);
    }
    // normalise weights 
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    for (auto& w : weights) w /= sum;

    // compute CDF
    std::vector<double> cdf(N);
    cdf[0] = weights[0];
    for (int i = 1; i < N; ++i)
        cdf[i] = cdf[i - 1] + weights[i];
    // Sample indices
    std::vector<int> sampled;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    while (sampled.size() < sampleCount)
    {
        double r = dis(gen);
        auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
        int idx = std::distance(cdf.begin(), it);
        sampled.push_back(idx);
    }
    return sampled;
}


std::vector<int> FlexSample::getSoftestModeIndices(const Eigen::VectorXf& singularValues)
{
    int totalModes = static_cast<int>(singularValues.size());
    std::vector<int> indices(totalModes);
    for (int i = totalModes -1; i >= 0; i--)
    {
        if (singularValues[i] < 1e-6)
        {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<glm::vec3> Flexibility::makePosVec(const AtomVector &atoms)
{
    std::vector<glm::vec3> positions;
    positions.reserve(atoms.size());
    for (Atom *atom : atoms) 
    {
        // std::cout << positions.size() << " is atom " << atom->desc() << std::endl;
        glm::vec3 pos = atom->derivedPosition();
        positions.push_back(pos);
    }
    return positions;
}

std::vector<float> Flexibility::makeRadiiVec(const AtomVector &atoms)
{
    std::vector<float> radii;
    radii.reserve(atoms.size());
    for (Atom *atom : atoms) 
    {
        // Add van der Waals radius
        std::string symbol = atom->elementSymbol();
        gemmi::Element ele = gemmi::Element(symbol);
        radii.push_back(ele.vdw_r());
    }
    return radii;
}


std::set<std::pair<int,int>> Flexibility::makeExcList(OpSet<Atom*> &atom_set)
{
    std::set<std::pair<int,int>> exclude;
    std::map<Atom*, int> indexing = atom_set.indexing();
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    auto check_bondstraint = [&exclude, &indexing]<class Type>(Type *b, Atom *left_atom)
    {
        int atomCount = b->atomCount();
        int left_idx = indexing[left_atom]; 
        for (int k = 0; k < atomCount; ++k)
        {
            Atom *right_atom = b->atom(k);
            if (right_atom == left_atom) continue;
            int right_idx = indexing[right_atom];
            auto key = std::minmax(left_idx, right_idx);
            exclude.insert(key);
            // Debug print
            // std::cout << "[makeExcList] Excluding pair: " 
            //           << key.first << " - " << key.second << "\n";
        }
    };

    for (Atom *left_atom : orderedAtoms)
    {
        // go throught all the Bondstraints
        int torsionCount = left_atom->bondTorsionCount();
        // for each torsion involving left_atom 
        for (int t = 0; t < torsionCount; ++t)
        {
            BondTorsion *torsion = left_atom->bondTorsion(t);
            check_bondstraint(torsion, left_atom);
        }
        int angleCount = left_atom->bondAngleCount();
        // for each torsion involving left_atom 
        for (int t = 0; t < angleCount; ++t)
        {
            BondAngle *angle = left_atom->bondAngle(t);
            check_bondstraint(angle, left_atom);
        }

    }
    std::set<std::pair<int,int>> hbond_exclusions = makeExcHBonds(orderedAtoms, indexing);
    exclude.insert(hbond_exclusions.begin(), hbond_exclusions.end());

    return exclude;
}

std::set<std::pair<int,int>> Flexibility::makeExcHBonds(std::vector<Atom*> orderedAtoms, std::map<Atom*, int> indexing)
{
    std::set<std::pair<int,int>> excludeHAtom;
    // std::map<Atom*, int> indexing = atom_set.indexing();

    // std::vector<Atom*> orderedAtoms = atom_set.toVector();
    std::vector<glm::vec3> positions = makePosVec(orderedAtoms);
    int n = positions.size();

    
    // Typical covalent N-H ~1.0 Å, O-H ~0.96 Å. Use a small margin.
    const float OH_cutoff = 0.96f; // 1.20f;
    const float NH_cutoff = 1.00f; //1.20f; 

    for (int i = 0; i < n; ++i)
    {
        Atom *a_i = orderedAtoms[i];
        if (a_i->elementSymbol() != "H") continue;

        for (int j = 0; j < n; ++j)
        {
            if (i == j) continue; 
            Atom *a_j = orderedAtoms[j];
            std::string elem = a_j ->elementSymbol();
            if (elem != "O" && elem !="N") continue;
            // choose cutoff based on heavy atom type
            float cutoff = (elem == "O") ? OH_cutoff : NH_cutoff;
            // axis-aligned early exits
            float dx = positions[i].x - positions[j].x;
            if (std::abs(dx) > cutoff) continue;
            float dy = positions[i].y - positions[j].y;
            if (std::abs(dy) > cutoff) continue;
            float dz = positions[i].z - positions[j].z;
            if (std::abs(dz) > cutoff) continue;


            // full squared-distance check
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 <= cutoff * cutoff)
            {
                auto key = std::minmax(indexing[a_i], indexing[a_j]);
                excludeHAtom.insert(key);
            }
        }
    }
    return excludeHAtom;
}



bool Flexibility::checkClashes(const std::vector<Atom*> orderedAtoms, 
                                int saved,
                               const std::vector<float> &radii,
                               const std::set<std::pair<int,int>> &exclude,
                               float tolerance)
{
    std::vector<glm::vec3> positions = makePosVec(orderedAtoms);
    using Pair = std::pair<int, int>;
    std::set<Pair> skip; 
    for (const auto &p : exclude) 
    { 
        skip.insert(std::minmax(p.first, p.second)); 
    }
    int n = positions.size();
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            // std::cout << "[checkClashes] Clash between atoms "
            //               << i << " and " << j << std::endl;

            if (skip.count(std::minmax(i, j)) > 0) continue; 
            float limit = radii[i] + radii[j] - tolerance;
            // float collisionFactor = 0.85f; // or 0.8–0.9, tune as needed
            // float limit = collisionFactor * (radii[i] + radii[j]);

            if (limit < 0.0f) continue; //radii to small

            // caclulate coordinate differences
            float  dx = positions[i].x - positions[j].x;
            if (std::abs(dx) > limit) continue; // early exit 

            float  dy = positions[i].y - positions[j].y;
            if (std::abs(dy) > limit) continue; // early exit 

            float dz = positions[i].z - positions[j].z;
            if (std::abs(dz) > limit) continue; // early exit 

            // only now compute the squared distance
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 > limit * limit) continue;

            Atom *left = orderedAtoms[i];
            Atom *right = orderedAtoms[j];
            glm::vec3 left_initial = left->otherPosition("other");
            glm::vec3 right_initial = right->otherPosition("other");
            // calculate dist between left_initial and right_intitila
            float dist_initial =  glm::length(left_initial - right_initial);
            if (dist_initial - tolerance < limit)
            {
                limit = dist_initial - tolerance;
            }
            if (dist2 > limit * limit) continue;

            std::cerr << "[checkClashes] Clash between atoms " << i << " and " << j << "\n";
            std::cerr << "[checkClashes] dist2 " << dist2 << " and limit " << limit << "\n";
            listClashes("list_clashes.csv", saved, orderedAtoms, i, j, radii);
            return false;
            
        }
    }
    return true;

}


void Flexibility::listClashes(const std::string &filename,
                              int saved,
                              const std::vector<Atom*> &orderedAtoms,
                              int i, int j,
                              const std::vector<float> &radii)
{
    std::ofstream file(filename, std::ios::app); // append mode
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header if this is the first clash for this sample
    file << "sample_structure_wo_vdW" << saved << "\n";

    Atom *a1 = orderedAtoms[i];
    Atom *a2 = orderedAtoms[j];

    file << a1->desc() << "," 
         << std::fixed << std::setprecision(3) << radii[i] << ","
         << a2->desc() << ","
         << std::fixed << std::setprecision(3) << radii[j] << "\n";

    file.close();
}


