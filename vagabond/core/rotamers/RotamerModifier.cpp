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
#include <vagabond/core/files/CsvFile.h>


RotamerModifier::RotamerModifier(Instance *inst)
{
    setInstance(inst);
    setup();
}

void RotamerModifier::setup()
{
    _instance->load();
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
    _instance->unload();
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
        r->transplantPositions(false);
    }
    r->transplantPositions(false);
    r->destroy();
    return weight;
}

void RotamerModifier::move(float weight, parameter xy)
{
    Result *r = new Result;
    glm::mat4x4 transformation = glm::mat4x4(1.0f);
    glm::vec3 translation {0.f, 0.f, 0.f};
    if (xy == MoveX && weight != _memoryX)
    {
        translation.y = weight-_memoryX;
        transformation = glm::translate(transformation, translation);
        _memoryX = weight;
        r->aps = _bouquet->move(transformation, "B");
        r->transplantPositions(false);
        r->destroy();
    }
    if (xy == MoveY && weight != _memoryY)
    {
        translation.z = weight-_memoryY;
        transformation = glm::translate(transformation, translation);
        _memoryY = weight;
        r->aps = _bouquet->move(transformation, "B");
        r->transplantPositions(false);
        r->destroy();
    }
}
void RotamerModifier::analysis(int timePoints, std::vector<glm::vec3> startPos)
{
    std::map<std::pair<ResidueId, std::string>, std::map<int, int> > collisionTracking;
    int iteration{0};
    for (auto translation: startPos)
    {
        std::cout << "Number of startPos: " <<startPos.size() << std::endl;
        for (int x = timePoints; x > 0; x--)
        {
            glm::mat4x4 transformation = glm::mat4x4(1.0f);
            transformation = glm::translate(transformation, translation / glm::vec3(timePoints));
            _bouquet->move(transformation, "B");
            std::vector<std::pair<ResidueId, std::string> > list{_bouquet->collisionList()};
            for (auto &member: list)
            {
                collisionTracking[member][iteration] ? collisionTracking[member][iteration] += 1 : collisionTracking[member][iteration] = 1;
            }
        }
        _bouquet->move(glm::translate(glm::mat4x4(1.0f), -translation), "B");
        iteration +=  1;
    }
    for (auto element: collisionTracking)
    {
        std::cout << std::endl << "Number of results: "<< element.second.size() << std::endl;
        std::cout << std::endl << "Residue  " << element.first.first << " collided";
        for (int x = 0; x < startPos.size(); x++)
            std::cout << '\t' << (element.second[x] ? element.second[x] : 0);
    }
}
void RotamerModifier::analysisTest(int timePoints, std::vector<glm::vec3> startPos)
{
    std::vector<Bouquet *> resChainA {_bouquet->bouquetsForChain("A")};
    std::vector<Bouquet *> resChainB {_bouquet->bouquetsForChain("B")};
    std::vector<std::string> resChainAstr {};
    for (Bouquet* const &bouquet : resChainA)
    {
        for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
            resChainAstr.push_back(bouquet->name().first.as_string()+std::to_string(rotamers));
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
    for (auto translation: startPos)
    {
        Eigen::MatrixXi topQuality(sizeA,sizeB);
        topQuality.fill(0);
        std::cout << "startPos number: " << iteration+1 << '/' << startPos.size() << std::endl;
        glm::mat4x4 transformation = glm::mat4x4(1.0f);
        transformation = glm::translate(transformation, translation / glm::vec3(timePoints+1));
        for (int x = 0; x <= timePoints; x++)
        {

            // _bouquet->move(transformation, "B");
            Eigen::MatrixXi currentCollisions {_bouquet->fullCollisionChecks(resChainA,resChainB, sizeA, sizeB)};
            topQuality += currentCollisions;
            std::vector<float> RotamersA {};
            std::vector<float> RotamersB {};
            int line {0};
            int row {0};
            for (auto const &bouquet : resChainA)
            {
                float collisions {0};
                for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
                {
                    if (currentCollisions.row(line).sum() != 0)
                        collisions++;
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
                    if (currentCollisions.col(row).sum() != 0)
                        collisions++;
                    row++;
                }
                RotamersB.push_back(collisions/static_cast<float>(bouquet->storeSize()));
            }
            CollidingRotamersB[iteration].push_back(RotamersB);
            _bouquet->move(transformation, "B");
        }
        _bouquet->move(glm::translate(glm::mat4x4(1.0f), -translation), "B");
        allAnalysis.push_back(topQuality);
        iteration++;
    }
    for (int iter = 0; iter < startPos.size(); iter++)
    {
        std::string fileName = _instance->entity_id() + "_paths_number_" + std::to_string(iter) + '-' + std::to_string(timePoints) + ".csv";
        std::string csvContent {""};
        std::cout << std::endl << "ITERATION " << iter << std::endl <<'\t';
        for (auto const &bouquet : resChainB)
        {
            for (int rotamers = 0; rotamers < bouquet->storeSize(); rotamers++)
            {
                csvContent += bouquet->name().first.as_string() + std::to_string(rotamers) + ',';
            }
        }
        csvContent += '\n';
                    for (auto element : resChainAstr)
            {
                csvContent += element + ',';
            }
        for (int rotamerA = 0; rotamerA < sizeA; rotamerA++)
        {
            csvContent += '\n';
            for (int rotamerB = 0; rotamerB < sizeB; rotamerB++)
            {
                csvContent += std::to_string(allAnalysis[iter](rotamerA,rotamerB)) + ',';
            }
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
        std::string fileName = _instance->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotA" + ".csv";
        std::string csvContent {""};
        for (auto const &bouquet : resChainA)
        {
                csvContent += bouquet->name().first.as_string() + ',';
        }
        csvContent += '\n';
        for (auto vector : pairs.second)
        {
            for (auto value : vector)
            {
                csvContent += std::to_string(value) + ',';
            }
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
        std::string fileName =  _instance->entity_id() + "_paths_number_" + std::to_string(pairs.first) + '-' + std::to_string(timePoints) + "availRotB" + ".csv";
        std::string csvContent {""};
        for (auto const &bouquet : resChainB)
        {
            csvContent += bouquet->name().first.as_string() + ',';
        }
        csvContent += '\n';
        for (auto vector : pairs.second)
        {
            for (auto value : vector)
            {
                csvContent += std::to_string(value) + ',';
            }
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
    _group = _instance->currentAtoms();
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

void RotamerModifier::saveStructure(std::string name)
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
                        // if (_mode == Slider) //Former function to test sliding through all the rotamers
                        // {
                        //     rotamerNumber = get(0) * (_lib->_allRotamers[resName].size()-1.f);
                        //     _RotMem[idx].loaded = false;
                        //     _RotMem[idx].RotamerValue = rotamerNumber;
                        //     float targetTorsion = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'];
                        //     return targetTorsion - initialTorsion;
                        // }
                        if (_mode == Reset)
                        {
                            _RotMem[idx].RotamerValue = 0;
                            _RotMem[idx].loaded = true;
                            return _RotMem[idx].initialAngle-initialTorsion;
                        }
                        if (_mode == Map)
                        {
                            rotamerNumber = get(0);
                            if (rotamerNumber > _lib->_allRotamers[resName].size()-1 || rotamerNumber < 0)
                            {
                                return 0.f;
                            }
                            float targetTorsion = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'];
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
 * - generate a plan perpendicular to this helix' axis
 * - create two vectors that will enable movement of the moving helix
 */
{
    std::cout << "Number of atom in AtomGroup: " << _group->size() << " and number of atoms in Bouquet: " << _bouquet->numAtom() << std::endl;
    std::vector<glm::vec3> axisTemp = axisForChain("A");
    _axisMain = (axisTemp[1] - axisTemp[0]);
     std::vector<glm::vec3> axisTemp2 = axisForChain("B");
    _axisSecondary = (axisTemp2[1] - axisTemp2[0]);
    _normal = glm::normalize(_axisMain); //_normal == axis of the A chain
    _x = glm::normalize(cross(_normal, (axisTemp2[0]+axisTemp2[1])/glm::vec3(2.f)-(axisTemp[0]+axisTemp[1])/glm::vec3(2.f)));
    _y = glm:: normalize(cross(_normal, _x));
    if (_referential == false) // Rotating the helices to be aligned with the main referential
    {
        glm::mat3x3 R {_normal,_x,_y};
        R = glm::inverse(R);
        _normal = R * _normal;
        _instance->currentAtoms()->do_op([R](Atom *atom)
        {
            updatingAtomGroupPositions(R, atom);
        });
        _bouquet->move(R);
        _x = R * _x;
        _y = -(R * _y);
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
    std::vector<glm::vec3> points = _bouquet->axis("A",true);
    std::vector<glm::vec3> points2 = _bouquet->axis("B",true);
    points.push_back(points2[0]);
    points.push_back(points2[1]);
    return points;
}
std::vector<glm::vec3> RotamerModifier::drawAxis()
{
    return {_normal,_x,_y};
}

std::vector<glm::vec3> RotamerModifier::RandStartPos(int NumberPos)
{
    std::vector<glm::vec3> pos {};
    for (int x = 0; x < NumberPos; x++)
    {
        glm::vec3 newVec {};
        newVec.x = RandGen();
        newVec.y = RandGen();
        newVec.z = -std::abs(RandGen());
        pos.push_back(glm::normalize(newVec)*glm::vec3(10));
        std::cout << newVec << std::endl;
    }
    return pos;
}

float RotamerModifier::RandGen()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(-1000, 1000);
    const float random_number = dis(gen);
    std::cout << random_number << std::endl;
    return random_number;
}