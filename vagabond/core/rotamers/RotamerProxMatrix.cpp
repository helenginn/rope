//
// Created by romain on 25/09/2026.
//

#include "RotamerProxMatrix.h"

#include <fstream>


#include "vagabond/core/BondCalculator.h"
#include "vagabond/core/BondSequenceHandler.h"
#include "vagabond/core/engine/Task.h"
#include "vagabond/core/files/File.h"
#include <vagabond/core/Result.h>

#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/LocalMotion.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>

#include <vagabond/core/rotamers/Rotamers.h>

#include <vagabond/core/AtomPosMap.h>

RotMatrix::RotMatrix(std::map<std::string,std::vector<Rotamer>> Rotamers): _allRotamers(Rotamers)
{
    proximityMatrix();
}
void RotMatrix::proximityMatrix()
// Plan:
// Hijacking the RotamerMap function to generate a peptide with all the amino acids and all of their rotamers, then using their pos to do the calculations of the differences
// From there: using the min (0) =keeping the same rotamer and max (being the biggest movement (over all the aa?) to determine a likelyhood of this transformation

// 1. opening a file for an aminoAcid using ??

// 2.
{
    std::map<std::string, std::map<Atom*, std::vector<glm::vec3>>> mapPositionsSidechain {};

    for (auto const &residue : _allRotamers)
    {
        File *file = File::loadUnknown("./AminoAcids/" + residue.first +".pdb");
        _group = file->atoms();
        prepareResources();
        unifiedTorsionFetcher();

            std::cout << "/aminoAcids/" << residue.first << " is open" << std::endl;
            for (int x{0}; x < residue.second.size(); x++)
            {
                submitJob(x);
                Result *r = _resources.calculator->acquireObject();
                AtomPosList list = r->apl;
                for (auto const &atomWithPos: list)
                {
                    mapPositionsSidechain[residue.first][atomWithPos.atom].push_back(atomWithPos.wp.ave);
                }
                r->destroy();
            }
    }
    for (auto const &residue : mapPositionsSidechain)
    {
        std::string fileName = residue.first + "Proximity.csv";
        std::string csvContent {};
        Eigen::MatrixXf actualMatrix(residue.second.begin()->second.size(),residue.second.begin()->second.size());
        for (int firstPos {0}; firstPos < residue.second.begin()->second.size(); firstPos++)
        {
            for (int secondPos {0}; secondPos < residue.second.begin()->second.size(); secondPos++)
            {
                float totalLength {0};
                for (auto atomPos : mapPositionsSidechain[residue.first])
                {
                    totalLength += abs(length(atomPos.second[firstPos]-atomPos.second[secondPos]));
                    std::cout << '\n' << totalLength << std::endl;
                }
                csvContent += std::to_string(totalLength) + ',';
                std::cout << "End of " << residue.first << " rotamer number pair (" << firstPos << ", " << secondPos << std::endl;
                actualMatrix(firstPos, secondPos) = totalLength;
            }
            csvContent = csvContent.substr(0, csvContent.length()-1);
            csvContent += '\n';
        }
        std::ofstream file;
        file.open(fileName);
        if (file.is_open())
        {
            file << csvContent;
            file.close();
        }
        _proxMatrices.push_back(std::pair(residue.first, actualMatrix));
    }
    for (auto [name, matrix] : _proxMatrices)
    {
        Eigen::MatrixXi top5 {matrix.rows(),matrix.cols()};
        top5.fill(0);
        std::string csvContent {};
        for (int line = 0; line < matrix.rows(); line++)
        {
            Eigen::VectorXf valueOrdered {matrix.rows()};
            valueOrdered.fill(FLT_MAX);
            std::vector<float> valueOrderedTest {};
            valueOrderedTest.push_back(FLT_MAX);
            for (int pos =0; pos < matrix.cols(); pos++) // exponent of negative of square + scaling based onBoltzmann distribution
            {
                for (int min = 0; min < matrix.cols();min++)
                {
                    if (matrix(line,pos) < valueOrderedTest[min])
                    {
                        top5.row(line).array() = (top5.row(line).array() > min).select(top5.row(line).array() + 1, top5.row(line).array());
                        top5(line,pos) = min+1;
                        valueOrderedTest.emplace( valueOrderedTest.begin()+min, matrix(line, pos));
                        std::cout << " test" << std::endl;
                        break;
                    }
                }
            }
        }
        std::string fileName {name +"top5.csv"};
        for (int lines = 0; lines < top5.rows(); lines++)
        {
            for (int cols = 0; cols < top5.cols(); cols++)
            {
                csvContent+= std::to_string(top5(lines,cols)) + ',';
            }
            csvContent = csvContent.substr(0, csvContent.length()-1);
            csvContent += '\n';
        }
        std::ofstream file;
        file.open(fileName);
        file << csvContent;
        file.close();
    }
    std::cout << "terminado?" << std::endl;

}
std::map<std::string,Eigen::MatrixXf> RotMatrix::getPossibleRotamers()
{
    std::map<std::string,Eigen::MatrixXf> availRot {};
    if (_proxMatrices.empty())
        proximityMatrix();
    for (auto [name, matrix] : _proxMatrices)
    {
        Eigen::MatrixXf mat {};
        //mat = matrix.cast<int>();
        mat.array() = (matrix.array() < 10).cast<float>();
        availRot[name] = mat;
        std::cout << name << " matrix done" << std::endl;
    }
    return availRot;
}


void RotMatrix::prepareResources()
{
    const int threads = 1;
    _resources.allocateMinimum(threads);
    // set up per-bond/atom calculation
    _group->recalculate();
    std::vector<AtomGroup *> subsets = _group->connectedGroups();
    for (AtomGroup *subset : subsets)
    {
        Atom *anchor = subset->chosenAnchor();
        _resources.sequences->addAnchorExtension(anchor);
    }
    _resources.sequences->setIgnoreHydrogens(true);
    _resources.sequences->setup();
    _resources.sequences->prepareSequences();
    _params =
   _resources.sequences->torsionBasis()->parameters();
}
void RotMatrix::submitJob(float weight)
{
    BaseTask *first_hook = nullptr; // Initialize first hook
    CalcTask *final_hook = nullptr; // Initialize final hook

    CalcTask *calc_hook = nullptr; // Initialize calc hook
    Task<BondSequence *, void *> *let_sequence_go = nullptr; // Initialize let_sequence_go

    BondCalculator *const &calculator = _resources.calculator; // Gets the calculator
    BondSequenceHandler *sequences = _resources.sequences; // Gets the sequences

    /* this final task returns the result to the pool to collect later */
    Task<Result, void *> *submit_result = calculator->actOfSubmission(0); // Submits the result
    Flag::Calc calc = Flag::Calc(Flag::DoTorsions /*|Flag::DoSuperpose*/); // Sets calculation flags

    sequences->calculate(calc, {weight}, &first_hook, &final_hook); // Calculates sequences

    BondSequence* firstSequence = sequences->sequence(); // Gets the first sequence
    Flag::Extract gets = Flag::Extract(Flag::AtomVector); // Sets extraction flags

    let_sequence_go = sequences->extract(gets, submit_result, final_hook); // Extracts data
    _resources.tasks->addTask(first_hook); // Adds task to the task list
}

void RotMatrix::unifiedTorsionFetcher()
{
    CoordManager* coordManager = _resources.sequences->manager();
    auto sideChainPlusX = [this](const Coord::Get &get, const int &idx)
    {
        if (_params[idx]->isTorsion())
        {
            BondTorsion *torsion = dynamic_cast<BondTorsion *>(_params[idx]);
            if (!_params[idx]->coversMainChain())
            {
                float initialTorsion = torsion->refinedAngle();
                std::string resName = _params[idx]->owningAtom()->code();
                if (torsion->shortDesc().substr(0,3) == "chi")
                {
                    int rotamerNumber = 0;
                    {
                        rotamerNumber = get(0);
                        if (rotamerNumber < 0 || rotamerNumber >= _allRotamers[resName].size())
                        {
                            return 0.f;
                        }
                        Rotamer const &rota = _allRotamers[resName][rotamerNumber];
                        if (torsion->shortDesc()[3]-'0' > rota.chi.size())
                            return 0.f;
                        float targetTorsion = rota.chi[torsion->shortDesc()[3]-'1'];
                        return targetTorsion - initialTorsion;
                    }
                }
            }
        }
        return 0.f;
    };
    coordManager->setTorsionFetcher(sideChainPlusX);
}