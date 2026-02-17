#include "FlexAnalysis.h"
#include <vagabond/core/Flexibility.h>
#include <vagabond/core/Instance.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

#include "AtomGroup.h"


FlexAnalysis::FlexAnalysis(Flexibility *flex, Instance *instance) 
: _flex(flex), _instance(instance), _flexTag("flexPos")
{
	if (_flex == nullptr)
    {
        throw std::runtime_error("FlexAnalysis requires a valid Flexibility pointer");
    }
}

void FlexAnalysis::generateAtomCloud(int minCol, int maxCol, std::string flexTag)
{
    _flexTag = flexTag;
    const AtomVector &atoms = _instance->currentAtoms()->atomVector();

    for (Atom *atom : atoms)
    {
        atom->removeOtherPosition(_flexTag); 
    }

    std::vector<float> weights = { -3.0f, 0.0f, 3.0f };
    for (float weight : weights)
    {
        atomCloud(minCol, maxCol, weight, atoms);
    }

    std::cout << "Samples in first atom: "
          << atoms.front()->otherPositions(_flexTag).samples.size()
          << std::endl;

    std::cout << "End of B-factor estimation!" << std::endl;
    std::cout << "*** Saving position to sampled_positions.csv... ***" << std::endl;
    savePositionsToCSV("sampled_positions.csv", atoms);
    std::cout << "Average positions" << std::endl;
    calculateAnisoBfactors(atoms);
    saveBfactorsToCSV("bfactors.csv", atoms);
}


void FlexAnalysis::atomCloud(int minCol, int maxCol, float weight, const AtomVector &atoms)
{
    // change here witht the values that you get from the FlexView (give by the user)

    for (int i = minCol; i <= maxCol; ++i)
    {
        // _colIdx = i;
        _flex->submitJobAndRetrieve(weight);
        for (Atom *atom : atoms)
        {
            glm::vec3 vec = atom->derivedPosition();
            atom->addOtherPosition(_flexTag, vec);   
        }
    }

}

void FlexAnalysis::savePositionsToCSV(const std::string &filename, const AtomVector &atoms) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header
    file << "Description,Element,ResidueID,AtomName,Chain,SampleIndex,X,Y,Z\n";
    for (Atom *atom : atoms)
        {
            const WithPos &positions = atom->otherPositions(_flexTag);
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
                     << pos.x << ","
                     << pos.y << ","
                     << pos.z << "\n";
            }
        }

    file.close();
    std::cout << "Saved sampled positions to " << filename << std::endl;
}

void FlexAnalysis::saveBfactorsToCSV(const std::string &filename, const AtomVector &atoms) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header
    file << "Description,Element,ResidueID,AtomName,Chain,"
         << "B11,B22,B33,B12,B13,B23\n";
    for (Atom *atom : atoms)
        {
            const Eigen::Matrix3f &cov = atom->otherAnisoBfactors(_flexTag);
            file << atom->desc() << ","
                 << atom->elementSymbol() << ","
                 << atom->residueId() << ","
                 << atom->atomName() << ","
                 << atom->chain() << ","
                 << std::fixed << std::setprecision(4)
                 << cov(0, 0) << "," << cov(1, 1) << "," << cov(2, 2) << ","
                 << cov(0, 1) << "," << cov(0, 2) << "," << cov(1, 2) << "\n";

        }

    file.close();
    std::cout << "Saved Bfactors to " << filename << std::endl;
}

void FlexAnalysis::calculateAnisoBfactors(const AtomVector &atoms)
{

    for (Atom *atom : atoms)
        {
            const WithPos &positions = atom->otherPositions(_flexTag);
            const std::vector<glm::vec3> &samples = positions.samples;
            if (samples.empty())
            {
                continue;
            }
            glm::vec3 sum(0.f);
            for (const glm::vec3 &pos : samples)
            {
                sum += pos;
            }
            glm::vec3 avg = sum / static_cast<float>(samples.size());
            atom->setDerivedPosition(avg);
            // calculate covariance matrix 
            Eigen::Matrix3f covMat = covariance(samples);
            // Output: average and covariance
            std::cout << atom->desc() << ","
                    << atom->elementSymbol() << ","
                    << atom->residueId() << ","
                    << atom->atomName() << ","
                    << atom->chain() << ","
                    << std::fixed << std::setprecision(3)
                    << "AVG: (" << avg.x << ", " << avg.y << ", " << avg.z << "), " << std::endl;
            Eigen::IOFormat cleanFmt(3, 0, ", ", "\n", "[", "]");
            std::cout << "COV:\n" << covMat.format(cleanFmt) << "\n";
            // Regularize the covariance matrix
            float epsilon = 0.01f; // in Å²
            // covMat += epsilon * Eigen::Matrix3f::Identity();
            Eigen::Matrix3f bFactorTens = 8 * M_PI * M_PI * covMat;
            std::cout << "B factor tenstor:\n" << bFactorTens.format(cleanFmt) << "\n";
            atom->setDerivedAnisoBfactors(bFactorTens);

        }

}


Eigen::Matrix3f FlexAnalysis::covariance(const std::vector<glm::vec3> &samples) const
{
    glm::vec3 mean(0.0f);
    for (const glm::vec3 &v : samples)
    {
        mean += v;
    }
    mean /= static_cast<float>(samples.size());

    Eigen::Matrix3f cov = Eigen::Matrix3f::Zero();
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
// check if svd has already been calculated adn _Vsize and _V has been assinged: 
// this is done in calculateFlexWeights, called in processMultipleHBonds
	const Eigen::MatrixXf& V = _flex->getV();
	const Eigen::MatrixXf& S = _flex->getS();

    if (V.size() == 0 || S.size() == 0)
    {
        std::cerr << "Error: SVD has not been computed. "
                  << "Please run calculateFlexWeights() or equivalent first." 
                  << std::endl;
    }

    std::cout << "Calculating free energies for " << V.size() << " modes..." << std::endl;

    int numModes = S.size();;
    std::vector<double> enthalpies(numModes);
    std::vector<double> entropies(numModes);
    std::vector<double> freeEnergies(numModes);

    for (int i = 0; i < numModes; ++i)
    {
        // pick up singular values for matrix S:
        double sigma_i = S(i);
        double enthalpy = computeEnthalpy(sigma_i);
        enthalpies[i] = enthalpy;

        std::vector<float> v_i = _flex->extractVColumn(V, i);
        double entropy = computeEntropy(v_i);
        entropies[i] = entropy;

        // calucate free energy
        double c_T = 1.0;
        freeEnergies[i] = enthalpy - c_T * entropy;
    }
    saveFreeEnergyCSV("enthalpy_entropy_energy.csv", enthalpies, entropies, freeEnergies);
}

double FlexAnalysis::computeEnthalpy(double sigma) const
{
    const Eigen::MatrixXf& S = _flex->getS();
    const double k = 3.24; // kcal/mol scaling factor
    double maxSingVal = S.maxCoeff();
    return (maxSingVal > 0.0) ? sigma / maxSingVal : 0.0;
}

double FlexAnalysis::computeEntropy(const std::vector<float>& v_i) const
{
    double sqSum = 0.0;
    std::vector<double> kappa(v_i.size());
    for (size_t j = 0; j < v_i.size(); j++)
    {
        kappa[j] = v_i[j] * v_i[j];
        sqSum += kappa[j];
    }

    if (sqSum > 0)
    {
        for (size_t j = 0; j < kappa.size(); j++)
        {
            kappa[j] /= sqSum;
        }
    }

    // Shannon entropy
    double eVal = 0.0;
    for (size_t j = 0; j < kappa.size(); j++)
    {
        if (kappa[j] > 0)
        {
            eVal += -kappa[j] * log(kappa[j]);
        }
    }

    // normalized collectivity measure: between [1/N, 1]
    return (1.0 / (double)kappa.size()) * exp(eVal);

}

void FlexAnalysis::saveFreeEnergyCSV(const std::string &filename,
                                    const std::vector<double> &enthalpies,
                                    const std::vector<double> &entropies,
                                    const std::vector<double> &freeEnergies) const
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

    // Write header
    file << "Mode,Enthalpy,Entropy,FreeEnergy\n";

    // Write values with 6 decimal places
    file << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < enthalpies.size(); ++i)
    {
        file << i << "," 
             << enthalpies[i] << "," 
             << entropies[i] << "," 
             << freeEnergies[i] << "\n";
    }

    file.close();
    std::cout << "Free energy data saved to " << filename << std::endl;
}
