//
// Created by romain on 11/05/2026.
//
#include <vagabond/utils/Eigen/Core>

#include <gemmi/cifdoc.hpp>
#include <vagabond/core/rotamers/RotamerModifier.h>
#include "vagabond/utils/AcquireCoord.h"

#include <vagabond/core/engine/Task.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/LocalMotion.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/engine/CoordManager.h>
#include <vagabond/core/rotamers/Rotamers.h>
#include <vagabond/core/Parameter.h>
#include <vagabond/core/AtomPosMap.h>

#include <random>
#include "RotamerBouquet.h"
#include <regex>


RotamerModifier::RotamerModifier(Instance *instMain, Instance *instSec, std::string &mainChain, std::string &secChain)
    : _instMain(instMain), _instSec(instSec), _mainChain(mainChain), _secChain(secChain)
{
    setup();
}

void RotamerModifier::setup()
{
    _instMain->load();
    _instSec->load();
    prepareResources();
    prepareMemory();
    unifiedTorsionFetcher();
    _lib = new RotamerLibrary;
    _bouquet = new Bouquet;
    generateRotamerMapPosition();
    makePlan();
}
RotamerModifier::~RotamerModifier()
{
    _instMain->unload();
    _instSec->unload();
}

float RotamerModifier::submitJobAndRetrieve(float weight, parameter a)
{
    _mode = a;
    submitJob(weight);
    Result *r = _resources.calculator->acquireObject();
    if (_mode == Map)
    {
        AtomPosList list = r->apl;
        for (auto const &atomWithPos: list)
        {
            if (weight <= _lib->_allRotamers[atomWithPos.atom->code()].size() - 1)
            {
                ResidueId resID{atomWithPos.atom->residueId().as_string() + atomWithPos.atom->chain()};
                _resBouquet[resID][atomWithPos.atom].push_back(atomWithPos.wp.ave);
            }
        }
        r->destroy();
        return weight;
    }
    r->transplantPositions(false);
    r->destroy();
    return weight;
}

void RotamerModifier::move(float weight, parameter xy)
{
    // Result *r = new Result;
    // glm::mat4x4 transformation = glm::mat4x4(1.0f);
    // glm::vec3 translation {0.f, 0.f, 0.f};
    // if (xy == MoveX && weight != _memoryX)
    // {
    //     translation.y = weight-_memoryX;
    //     transformation = glm::translate(transformation, translation);
    //     _memoryX = weight;
    //     r->aps = _bouquet->move(transformation, _secChain);
    //     r->transplantPositions(false);
    //     r->destroy();
    // }
    // if (xy == MoveY && weight != _memoryY)
    // {
    //     translation.z = weight-_memoryY;
    //     transformation = glm::translate(transformation, translation);
    //     _memoryY = weight;
    //     r->aps = _bouquet->move(transformation, _secChain);
    //     r->transplantPositions(false);
    //     r->destroy();
    // }
}

glm::vec3 RotamerModifier::analysisTest(int timePoints, std::vector<glm::vec3> startPos, int willIterate)
{
    // STEP 1: Initialisation
    std::vector<Bouquet *> resChainA {_bouquet->bouquetsForChain(_mainChain)};
    std::vector<Bouquet *> resChainB {_bouquet->bouquetsForChain(_secChain)};
    std::vector<std::string> resChainAstr {};
    for (Bouquet* const &bouquet : resChainA)
    {
        for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
            resChainAstr.push_back(bouquet->name().first.as_string()+std::to_string(rotamers));
    }
    std::vector<std::string> resChainBstr {};
    for (Bouquet* const &bouquet : resChainB)
    {
        for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
            resChainBstr.push_back(bouquet->name().first.as_string()+std::to_string(rotamers));
    }
    int sizeA {0};
    int sizeB {0};
    for (auto bouquet : resChainA)
    {
        sizeA += bouquet->storeSize();
    }
    for (auto bouquet : resChainB)
    {
        sizeB += bouquet->storeSize();
    }
    std::vector<Eigen::MatrixXi> allAnalysis {};
    std::map<int, std::vector<std::vector<float>>> CollidingRotamersA {};
    std::map<int, std::vector<std::vector<float>>> CollidingRotamersB {};
    int iteration {0};

    //STEP 2: iteration for every vectors given in input
    for (auto translation: startPos)
    {
        Eigen::MatrixXi totalCollisions(sizeA,sizeB);
        totalCollisions.fill(0);
        std::cout << "startPos number: " << iteration+1 << '/' << startPos.size() << std::endl;
        glm::mat4x4 transformation = glm::mat4x4(1.0f);
        transformation = glm::translate(transformation, translation / glm::vec3(timePoints+1));
        for (int x = 0; x <= timePoints; x++)
        {
            // currentCollisions is a matrix of every collision between every rotamers of chain A and chain B
            Eigen::MatrixXi currentCollisions {_bouquet->fullCollisionChecks(resChainA,resChainB, sizeA, sizeB)};
            totalCollisions += currentCollisions;
            std::vector<float> RotamersA {};
            std::vector<float> RotamersB {};
            int line {0};
            int row {0};
            for (auto const &bouquet : resChainA)
            {
                float collisions {0};
                for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
                {
                    float totColl {0.f};
                    float totNumOfRot {0.f};
                    if (currentCollisions.row(line).sum() != 0)
                    {
                        int pos {0};
                        float coll {0.f};
                        float numOfRot {0.f};
                        std::string bouquetB {};
                        std::regex rgx("\\d+[A-Z]");
                        for (auto values : currentCollisions.row(line))
                        {
                            std::string const &resi = resChainBstr[pos];
                            std::smatch match;
                            std::regex_search(resi, match, rgx);
                            if (bouquetB != match[0])
                            {
                                if (coll != 0)
                                {
                                    totColl += coll;
                                    totNumOfRot += numOfRot;
                                }
                                numOfRot = 1;
                                (values == 1? coll = 1 : coll = 0);
                                bouquetB = match[0];
                            }
                            else
                            {
                                (values == 1? coll += 1 : coll += 0);
                                numOfRot += 1;
                            }
                            pos++;
                        }
                        totColl+=coll;
                        totNumOfRot+=numOfRot;
                        collisions += totColl/totNumOfRot;
                    }
                    line++;
                }
                RotamersA.push_back(collisions/static_cast<float>(bouquet->storeSize()));
            }
            CollidingRotamersA[iteration].push_back(RotamersA);
            for (auto const &bouquet : resChainB)
            {
                float collisions {0};
                for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
                {
                    float totColl {0.f};
                    float totNumOfRot {0.f};
                    if (currentCollisions.col(row).sum() != 0)
                    {
                        int pos {0};
                        float coll {0.f};
                        float numOfRot {0.f};
                        std::string bouquetA {};
                        std::regex rgx("\\d+[A-Z]");
                        for (auto values : currentCollisions.col(row))
                        {
                            std::string const &resi = resChainAstr[pos];
                            std::smatch match;
                            std::regex_search(resi, match, rgx);
                            if (bouquetA != match[0])
                            {
                                if (coll != 0)
                                {
                                    totColl += coll;
                                    totNumOfRot += numOfRot;
                                }
                                numOfRot = 1;
                                (values == 1? coll = 1 : coll = 0);
                                bouquetA = match[0];
                            }
                            else
                            {
                                (values == 1? coll += 1 : coll += 0);
                                numOfRot += 1;
                            }
                            pos++;
                        }
                        totColl+=coll;
                        totNumOfRot+=numOfRot;
                        collisions += totColl/totNumOfRot;
                    }
                    row++;
                }
                RotamersB.push_back(collisions/static_cast<float>(bouquet->storeSize()));
            }
            CollidingRotamersB[iteration].push_back(RotamersB);
            _bouquet->moveNoMap(transformation, _secChain);
        }
        _bouquet->moveNoMap(glm::translate(glm::mat4x4(1.0f), -translation), _secChain);
        allAnalysis.push_back(totalCollisions);
        iteration++;
    }
    for (int iter = 0; iter < startPos.size(); iter++)
    {
        std::string fileName {};
        if (willIterate ==0)
            fileName = "FINAL_" + _instMain->entity_id() + "_paths_number_" + std::to_string(iter) + '-' + std::to_string(timePoints) + ".csv";
        else
            fileName = "iter" + std::to_string(willIterate) + "_" + _instMain->entity_id() + "_paths_number_" + std::to_string(iter) + '-' + std::to_string(timePoints) + ".csv";

        std::string csvContent {};
        csvContent += startPos[iter].x + ',' + startPos[iter].y + ',' + startPos[iter].z + '\n';
        for (auto const &bouquet : resChainB)
        {
            for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
            {
                csvContent += bouquet->name().first.as_string() + std::to_string(rotamers) + ',';
            }
        }
        csvContent = csvContent.substr(0, csvContent.length()-1);
        csvContent += '\n';
                    for (auto const & element : resChainAstr)
            {
                csvContent += element + ',';
            }
        csvContent = csvContent.substr(0, csvContent.length()-1);
        for (int rotamerA = 0; rotamerA < sizeA; rotamerA++)
        {
            csvContent += '\n';
            for (int rotamerB = 0; rotamerB < sizeB; rotamerB++)
            {
                csvContent += std::to_string(allAnalysis[iter](rotamerA,rotamerB)) + ',';
            }
            csvContent = csvContent.substr(0, csvContent.length()-1);
        }
        std::ofstream file;
        file.open(fileName);
        if (file.is_open())
        {
            file << csvContent;
            file.close();
        }
    }
    for (auto const &pairs : CollidingRotamersA)
    {
        std::string fileName {};
        if (willIterate == 0)
            fileName = "FINAL_" + _instMain->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotA" + ".csv";
        else
            fileName = "iter" + std::to_string(willIterate) + "_" + _instMain->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotA" + ".csv";

        std::string csvContent {};
        csvContent += std::to_string(startPos[pairs.first].x) + ',' + std::to_string(startPos[pairs.first].y) + ',' + std::to_string(startPos[pairs.first].z) + '\n';
        for (auto const &bouquet : resChainA)
        {
                csvContent += bouquet->name().first.as_string() + ',';
        }
        csvContent = csvContent.substr(0, csvContent.length()-1);
        csvContent += '\n';
        for (auto const &vector : pairs.second)
        {
            for (auto value : vector)
            {
                csvContent += std::to_string(value) + ',';
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
    }
    for (auto const &pairs : CollidingRotamersB)
    {
        std::string fileName {};
        if (willIterate == 0)
            fileName =  "FINAL_" + _instMain->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotB" + ".csv";
        else
            fileName =  "iter" + std::to_string(willIterate) + "_" + _instMain->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotB" + ".csv";

        std::string csvContent {};
        csvContent += std::to_string(startPos[pairs.first].x) + ',' + std::to_string(startPos[pairs.first].y) + ',' + std::to_string(startPos[pairs.first].z) + '\n';
        for (auto const &bouquet : resChainB)
        {
            csvContent += bouquet->name().first.as_string() + ',';
        }
        csvContent = csvContent.substr(0, csvContent.length()-1);
        csvContent += '\n';
        for (auto const &vector : pairs.second)
        {
            for (auto value : vector)
            {
                csvContent += std::to_string(value) + ',';
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
    }
    // Saving the vectors (hedgehog) and assigning their average collisionValues (to be able to display them with nice colors) these vectors are corrected to be in the right orientation of the initial model
    std::string fileNameSHH {};
    if (willIterate == 0)
        fileNameSHH = "FINAL_" + _instMain->entity_id() + "_hedgehog.csv";
    else
        fileNameSHH = "iter" + std::to_string(willIterate) + "_" + _instMain->entity_id() + "_hedgehog.csv";

    std::string csvContent{};
    csvContent += _instMain->currentAtoms()[0].chosenAnchor()->chain() + '\n';
    std::vector<glm::vec4> vectorsWeighted {};
    for (auto const &pairs : CollidingRotamersB)
    {
        std::vector<glm::vec4> vectorSHH {};

        std::vector<float> means {};
        for (auto const &vector : pairs.second)
        {
            float sumValues {0};
            float numberValues {0};
            for ( auto const &values : vector)
            {
                sumValues += values;
                numberValues += 1;
            }
            means.push_back(sumValues/numberValues);
        }
        float sumValues {0};
        float numberValues {0};
        for (auto values : means)
        {
            sumValues += values;
            numberValues += 1;
        }
        glm::mat4x4 reset = glm::inverse(_transform);
        reset[0][3]= 0.f;
        reset[1][3]= 0.f;
        reset[2][3]= 0.f;
        glm::vec3 currentVec {reset*glm::vec4(startPos[pairs.first], 0.f)};
        csvContent += std::to_string(currentVec.x) + ',' + std::to_string(currentVec.y) + ',' + std::to_string(currentVec.z) + ',' + std::to_string(sumValues/numberValues) + '\n';
        if (willIterate != 0)
            vectorsWeighted.push_back(glm::vec4(startPos[pairs.first], sumValues/numberValues));
    }
    std::cout << csvContent;
    std::ofstream file;
    file.open(fileNameSHH);
    if (file.is_open())
    {
        file << csvContent;
        file.close();
    }
    if (willIterate != 0)
        return minimumClashes(vectorsWeighted);
    else
        return glm::vec3(0,0,0);
}

void RotamerModifier::analysisPipeline(int timePoints, std::vector<glm::vec3> startPos, int iterations)
{
    while (iterations > 1)
    {
        glm::vec3 startPosIter {analysisTest(timePoints, startPos, iterations)};
        startPos = newStartPos(startPos.size(), startPosIter, iterations);
        iterations -=1;
    }
    analysisTest(timePoints, startPos);
}

glm::vec3 RotamerModifier::minimumClashes(std::vector<glm::vec4> vectorsWeighted)
{
    float sumOfWeight {0.f};
    glm::vec3 sumVectors {0.f};
    for (auto vectors : vectorsWeighted)
    {
        glm::vec3 current {glm::vec3(vectors)};
        float weight {1/vectors.w};
        current = glm::normalize(current);
        sumVectors += current*glm::vec3(weight);
        sumOfWeight += weight;
    }
    return  glm::normalize(sumVectors/sumOfWeight);
}

std::vector<glm::vec3> RotamerModifier::newStartPos(int numOfPos, glm::vec3 norm,int runNum)
{
    std::vector<glm::vec3> newPos {};
    glm::vec3 currentPos {};
    glm::vec3 helper = (glm::abs(norm.x) < 0.9f)
                           ? glm::vec3(1, 0, 0)
                           : glm::vec3(0, 1, 0);
    glm::vec3 ortho1 = glm::cross(norm, helper);
    glm::vec3 ortho2 = glm::cross(ortho1, norm);
    std::ofstream file {};
    std::string title {"Corrections_vectors_for_"+ (runNum >= 2 ? "iter"+std::to_string(runNum) : "FINAL" )+".csv" };
    std::string csvContent {};
    csvContent += std::to_string(norm.x) + ',' + std::to_string(norm.y) + ',' + std::to_string(norm.z) + '\n';
    csvContent += std::to_string(ortho1.x) + ',' + std::to_string(ortho1.y) + ',' + std::to_string(ortho1.z) + '\n';
    csvContent += std::to_string(ortho2.x) + ',' + std::to_string(ortho2.y) + ',' + std::to_string(ortho2.z);
    file.open(title);
    if (file.is_open())
        file << csvContent;
    file.close();
    for (int counter =0; counter < numOfPos; counter++)
    {
        currentPos = glm::normalize(norm*glm::vec3(RandGen(0,1000)) +ortho1*glm::vec3(RandGen(-1000,1000)) +ortho2*glm::vec3(RandGen(-1000,1000)))*glm::vec3(10);
        newPos.push_back(currentPos);
    }
    return newPos;
}
void RotamerModifier::submitJob(float weight)
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

void RotamerModifier::prepareMemory()
{
    for (int idx = 0; idx <= _params.size()-1; idx++)
    {
        if (_params[idx]->isTorsion())
        {
            BondTorsion *torsion = dynamic_cast<BondTorsion *>(_params[idx]);
            Rot = new RotamerMap;
            if (!_params[idx]->coversMainChain())
            {
                Rot->initialAngle  = torsion->refinedAngle();
                Rot->RotamerValue = 0;
                Rot->loaded = true;
                _RotMem[idx] = *Rot;
            }
        }
    }
}

void RotamerModifier::prepareResources()
{
    const int threads = 1;
    _resources.allocateMinimum(threads);
    // set up per-bond/atom calculation
    _group = _instMain->currentAtoms();
    _group->add(_instSec->currentAtoms());
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

void RotamerModifier::saveStructure(std::string const &name)
{
    AtomGroup *group = _instance->currentAtoms();
    group->writeToFile(name);
}

void RotamerModifier::unifiedTorsionFetcher()
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
                        if (_mode == Reset)
                        {
                            _RotMem[idx].RotamerValue = 0;
                            _RotMem[idx].loaded = true;
                            return _RotMem[idx].initialAngle-initialTorsion;
                        }
                        if (_mode == Map)
                        {
                            rotamerNumber = get(0);
                            if (rotamerNumber < 0 || rotamerNumber >= _lib->_allRotamers[resName].size())
                            {
                                return 0.f;
                            }
                            Rotamer const &rota = _lib->_allRotamers[resName][rotamerNumber];
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

void RotamerModifier::generateRotamerMapPosition()
{
    if (_map == false)
    {
        for (int x = -1; x <= 34; x++)
        {
            submitJobAndRetrieve(x, Map);
        }
        _bouquet->storeRotamers(_resBouquet);
        _bouquet->collisionUpdate();
        _resBouquet.clear();
    }
}

std::vector<glm::vec3> RotamerModifier::axisForChain(std::string const &chainName)
{
    return _bouquet->axis(chainName);
}

void RotamerModifier::makePlan()
/* Plan:
 * - take the reference helix (the one that will stay static)
 * - generate a plan perpendicular to this helix axis
 * - create two vectors that will enable movement of the moving helix
 */
{
    std::cout << "Number of atom in AtomGroup: " << _group->size() << " and number of atoms in Bouquet: " << _bouquet->numAtom() << std::endl;
    std::vector<glm::vec3> axisTemp = axisForChain(_mainChain);
    _axisMain = (axisTemp[1] - axisTemp[0]);
     std::vector<glm::vec3> axisTemp2 = axisForChain(_secChain);
    _axisSecondary = (axisTemp2[1] - axisTemp2[0]);
    _normal = glm::normalize(_axisMain); //_normal == axis of the A chain
    _y = glm::normalize(cross(_normal, (axisTemp2[0]+axisTemp2[1])/glm::vec3(2.f)-(axisTemp[0]+axisTemp[1])/glm::vec3(2.f)));
    _z = glm:: normalize(cross(_normal, _y));
    if (_referential == false) // Rotating the helices to be aligned with the main referential
    {
        glm::mat3x3 R {_normal,_y,_z};
        R = glm::inverse(R);
        glm::mat4x4 const R4 = glm::mat4(R);
        glm::mat4x4 T = glm::translate(glm::mat4(1.0f), -axisTemp[0]);
        _transform = R4 * T;
//         _instMain->currentAtoms()->do_op([transform](Atom *atom)
//         {
//             updatingAtomGroupPositions(transform, atom);
//         });
//         _instSec->currentAtoms()->do_op([transform](Atom *atom)
// {
//     updatingAtomGroupPositions(transform, atom);
// });
        _bouquet->moveNoMap(_transform);


        _y = R * _y;
        _z = -(R * _z);
        _normal = R * _normal;
        _axisMain = R * _axisMain;
        _axisSecondary = R * _axisSecondary;
        _referential = true;
        Result *r = new Result;
        r->aps = _bouquet->extractForGUI();
        r->transplantPositions();
        r->destroy();
        _map = true;
    }
}

void RotamerModifier::updatingAtomGroupPositions(glm::mat4x4 const &transformationMat, Atom* atom)
{
    if (atom != nullptr)
        atom->setDerivedPosition(glm::vec3(transformationMat*glm::vec4(atom->derivedPosition(),1.0f)));
}
std::vector<glm::vec3> RotamerModifier::drawChainAxis()
{
    std::vector<glm::vec3> points = _bouquet->axis(_mainChain);
    std::vector<glm::vec3> points2 = _bouquet->axis(_secChain);
    points.push_back(points2[0]);
    points.push_back(points2[1]);
    return points;
}
std::vector<glm::vec3> RotamerModifier::drawAxis()
{
    return {_normal,_y,_z};
}

std::vector<glm::vec3> RotamerModifier::RandStartPos(int const &numberPos)
{
    std::vector<glm::vec3> pos {};
    for (int x = 0; x < numberPos; x++)
    {
        glm::vec3 newVec {};
        newVec.x = RandGen(-1000, 1000);
        newVec.y = RandGen(0,1000);
        newVec.z = RandGen(-1000, 1000);
        pos.push_back(glm::normalize(newVec)*glm::vec3(10));
        std::cout << newVec << std::endl;
    }
    return pos;
}

int RotamerModifier::RandGen(int min, int max)
{
    std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    const int random_number = dis(gen);
    return random_number;
}