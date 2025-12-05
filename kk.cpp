// FlexAnalysis.cpp
#include "FlexAnalysis.h"
#include "Flexibility.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

#include "AtomGroup.h"   // for AtomVector, Atom, WithPos, etc.

FlexAnalysis::FlexAnalysis(Flexibility *flex)
: _flex(flex), _minCol(-1), _maxCol(-1)
{
    if (_flex == nullptr)
    {
        throw std::runtime_error("FlexAnalysis requires a valid Flexibility pointer");
    }
    _flexTag = "flexPos";
}

void FlexAnalysis::setColumnRange(int minCol, int maxCol)
{
    if (minCol > maxCol)
    {
        throw std::invalid_argument("minCol must be <= maxCol");
    }
    _minCol = minCol;
    _maxCol = maxCol;
}

void FlexAnalysis::generateAtomCloud()
{
    // get atoms from Flexibility via a getter on Flexibility (preferred)
    // If Flexibility exposes instance/currentAtoms, use that getter. Here we assume
    // Flexibility::instance() or similar — if not present, you can pass atoms in.
    AtomVector atoms = _flex->instance()->currentAtoms()->atomVector();

    // clear previous samples
    for (Atom *atom : atoms)
    {
        atom->removeOtherPosition(_flexTag);
    }

    // default weights if you wanted weight-driven sampling (kept for backward compat)
    std::vector<float> weights = { -3.0f, 0.0f, 3.0f };

    // We'll use column-based sampling: run through columns if range not set
    if (_maxCol < _minCol)
    {
        int vcols = _flex->getVcolumns();
        if (vcols <= 0)
        {
            std::cerr << "FlexAnalysis::generateAtomCloud: no V columns available.\n";
            return;
        }
        _minCol = 0;
        _maxCol = vcols - 1;
    }

    // For each column index, run the engine and store derived positions.
    for (int i = _minCol; i <= _maxCol; ++i)
    {
        // this method must be implemented in Flexibility:
        // - prepare torsion weights from column i
        // - submit tasks
        // - acquire Result and transplantPositions(...)
        _flex->submitJobAndRetrieveColumn(i);

        // now read derived positions and save them as otherPositions
        for (Atom *atom : atoms)
        {
            glm::vec3 vec = atom->derivedPosition();
            atom->addOtherPosition(_flexTag, vec);
        }
    }

    // Post-processing: CSV + B-factors
    if (!atoms.empty())
    {
        std::cout << "Samples in first atom: "
                  << atoms.front()->otherPositions(_flexTag).samples.size()
                  << std::endl;
    }

    savePositionsToCSV("sampled_positions.csv", _flexTag, atoms);
    calculateAnisoBfactors(_flexTag, atoms);
    saveBfactorsToCSV("bfactors.csv", _flexTag, atoms);
}

void FlexAnalysis::savePositionsToCSV(const std::string &filename,
                                      const std::string &tag,
                                      const AtomVector &atoms)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    file << "Description,Element,ResidueID,AtomName,Chain,SampleIndex,X,Y,Z\n";
    for (Atom *atom : atoms)
    {
        const WithPos &positions = atom->otherPositions(tag);
        for (size_t i = 0; i < positions.samples.size(); ++i)
        {
            const glm::vec3 &pos = positions.samples[i];
            file << atom->desc() << ","
                 << atom->elementSymbol() << ","
                 << atom->residueId() << ","
                 << atom->atomName() << ","
                 << atom->chain() << ","
                 << i << ","
                 << std::fixed << std::setprecision(3)
                 << pos.x << "," << pos.y << "," << pos.z << "\n";
        }
    }

    file.close();
    std::cout << "Saved sampled positions to " << filename << std::endl;
}

void FlexAnalysis::saveBfactorsToCSV(const std::string &filename,
                                     const std::string &tag,
                                     const AtomVector &atoms)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    file << "Description,Element,ResidueID,AtomName,Chain,B11,B22,B33,B12,B13,B23\n";
    for (Atom *atom : atoms)
    {
        const Eigen::Matrix3f &cov = atom->otherAnisoBfactors(tag);
        file << atom->desc() << ","
             << atom->elementSymbol() << ","
             << atom->residueId() << ","
             << atom->atomName() << ","
             << atom->chain() << ","
             << std::fixed << std::setprecision(4)
             << cov(0,0) << "," << cov(1,1) << "," << cov(2,2) << ","
             << cov(0,1) << "," << cov(0,2) << "," << cov(1,2) << "\n";
    }

    file.close();
    std::cout << "Saved Bfactors to " << filename << std::endl;
}

void FlexAnalysis::calculateAnisoBfactors(const std::string &tag, const AtomVector &atoms)
{
    for (Atom *atom : atoms)
    {
        const WithPos &positions = atom->otherPositions(tag);
        const std::vector<glm::vec3> &samples = positions.samples;
        if (samples.empty()) continue;

        glm::vec3 sum(0.f);
        for (const glm::vec3 &pos : samples) sum += pos;
        glm::vec3 avg = sum / static_cast<float>(samples.size());
        atom->setDerivedPosition(avg);

        Eigen::Matrix3f covMat = covariance(samples);
        Eigen::Matrix3f bFactorTens = 8.0f * static_cast<float>(M_PI * M_PI) * covMat;
        atom->setDerivedAnisoBfactors(bFactorTens);
    }
}

Eigen::Matrix3f FlexAnalysis::covariance(const std::vector<glm::vec3> &samples)
{
    Eigen::Matrix3f cov = Eigen::Matrix3f::Zero();
    if (samples.empty()) return cov;

    glm::vec3 mean(0.0f);
    for (const glm::vec3 &v : samples) mean += v;
    mean /= static_cast<float>(samples.size());

    for (const glm::vec3 &v : samples)
    {
        Eigen::Vector3f diff(v.x - mean.x, v.y - mean.y, v.z - mean.z);
        cov += diff * diff.transpose();
    }
    cov /= static_cast<float>(samples.size());
    return cov;
}

void FlexAnalysis::calculateFreeEnergy()
{
    // use the public getter for SVD
    SVDResult res = _flex->getSVD();
    if (res.V.size() == 0 || res.singularValues.size() == 0)
    {
        std::cerr << "Error: SVD has not been computed or is empty.\n";
        return;
    }

    int numModes = static_cast<int>(res.singularValues.size());
    std::vector<double> enthalpies(numModes);
    std::vector<double> entropies(numModes);
    std::vector<double> freeEnergies(numModes);

    double maxSingVal = res.singularValues.maxCoeff();

    for (int i = 0; i < numModes; ++i)
    {
        double sigma_i = static_cast<double>(res.singularValues(i));
        enthalpies[i] = computeEnthalpy(sigma_i, maxSingVal);

        // extract column i
        Eigen::VectorXf col = res.V.col(i);
        std::vector<float> v_i(col.data(), col.data() + col.size());

        entropies[i] = computeEntropy(v_i);
        double c_T = 1.0;
        freeEnergies[i] = enthalpies[i] - c_T * entropies[i];
    }

    saveFreeEnergyCSV("enthalpy_entropy_energy.csv", enthalpies, entropies, freeEnergies);
}

double FlexAnalysis::computeEnthalpy(double sigma, double maxSingVal)
{
    return (maxSingVal > 0.0) ? (sigma / maxSingVal) : 0.0;
}

double FlexAnalysis::computeEntropy(const std::vector<float>& v_i)
{
    if (v_i.empty()) return 0.0;

    double sqSum = 0.0;
    std::vector<double> kappa(v_i.size());
    for (size_t j = 0; j < v_i.size(); ++j)
    {
        kappa[j] = static_cast<double>(v_i[j]) * static_cast<double>(v_i[j]);
        sqSum += kappa[j];
    }

    if (sqSum > 0.0)
    {
        for (double &val : kappa) val /= sqSum;
    }

    double eVal = 0.0;
    for (double p : kappa)
    {
        if (p > 0.0) eVal += -p * std::log(p);
    }

    return (1.0 / static_cast<double>(kappa.size())) * std::exp(eVal);
}

void FlexAnalysis::saveFreeEnergyCSV(const std::string &filename,
                                     const std::vector<double> &enthalpies,
                                     const std::vector<double> &entropies,
                                     const std::vector<double> &freeEnergies)
{
    if (enthalpies.size() != entropies.size() || enthalpies.size() != freeEnergies.size())
    {
        std::cerr << "Error: Mismatched vector sizes for free energy data." << std::endl;
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    file << "Mode,Enthalpy,Entropy,FreeEnergy\n";
    file << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < enthalpies.size(); ++i)
    {
        file << i << "," << enthalpies[i] << "," << entropies[i] << "," << freeEnergies[i] << "\n";
    }

    file.close();
    std::cout << "Free energy data saved to " << filename << std::endl;
}
