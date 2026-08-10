//
// Created by romain on 11/05/2026.
//

#include <gemmi/cifdoc.hpp>
#include <vagabond/core/rotamers/RotamerModifier.h>

#include "vagabond/utils/AcquireCoord.h"

#include <vagabond/core/engine/Task.h>
#include <vagabond/core/TabulatedData.h>
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

#include "RotamerBouquet.h"
#include "vagabond/gui/elements/Line.h"


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
    _lib = new RotamerLibrary; // THIS was causing the issue in the first execution of the map (with missing atoms) when between instance->load() and prepareResources() and  no idea why though
    _bouquet = new Bouquet2;
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

        if (_mode == Map && !_map) //generating an AtomPosMap from the iteration of all the rotamers (to generate the multi-rotamers structure)
        {
            AtomPosList list = r->apl;
            for (auto const& atomWithPos : list)
            {
                if (weight <= _lib->_allRotamers[atomWithPos.atom->code()].size() - 1)
                {
                    ResidueId resID {atomWithPos.atom->residueId().as_string() + atomWithPos.atom->chain()};
                    _allRotamer.resBouquet[resID][atomWithPos.atom].push_back(atomWithPos.wp.ave);
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
    glm::mat4x4 transfo = glm::mat4x4(1.0f);
    glm::vec3 translation {0.f, 0.f, 0.f};
    if (xy == MoveX && weight != _memoryX)
    {
        translation.y = weight-_memoryX;
        transfo = glm::translate(transfo, translation);
        _memoryX = weight;
        r->aps = _bouquet->move(transfo, "A");
        r->transplantPositions(false);
        r->destroy();
    }
    if (xy == MoveY && weight != _memoryY)
    {
        translation.z = weight-_memoryY;
        transfo = glm::translate(transfo, translation);
        _memoryY = weight;
        r->aps = _bouquet->move(transfo, "A");
        r->transplantPositions(false);
        r->destroy();
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
            BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
            Rot = new RotamerMap;
            if (!_params[idx]->coversMainChain())
            {
                Rot->initialAngle  = torsion->refinedAngle();
                Rot->RotamerValue = 0;
                Rot->loaded = true;
                _RotamerMemory[idx] = *Rot;
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
                BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
                if (!_params[idx]->coversMainChain())
                {
                    float initialTorsion = torsion->refinedAngle();
                    std::string resName = _params[idx]->owningAtom()->code(); //get the 3 letters code of the bond's residue -> maybe bug there? (is it possible to get the wrong residue?)

                    if (torsion->shortDesc().substr(0,3) == "chi")
                    {
                        int rotamerNumber = 0;
                        if (_mode == Slider)
                        {
                            rotamerNumber = get(0) * (_lib->_allRotamers[resName].size()-1.f); // slider goes from 0 to 1, so this line enable going through all the rotamers for each residue, with the same slider range
                            _RotamerMemory[idx].loaded = false;
                            _RotamerMemory[idx].RotamerValue = rotamerNumber; // I let these commented line because it can help me debug the function
                            float targetTorsion = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0']; // don't like the operation of char substraction in chi[index]
                            return targetTorsion - initialTorsion;
                        }
                        if (_mode == Reset)
                        {
                            _RotamerMemory[idx].RotamerValue = 0;
                            _RotamerMemory[idx].loaded = true;
                            return _RotamerMemory[idx].initialAngle-initialTorsion;
                        }
                        if (_mode == Map)
                        {
                            rotamerNumber = get(0);
                            if (rotamerNumber > _lib->_allRotamers[resName].size()-1)
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

void RotamerModifier::generateRotamerMapPosition() //iteration for all the rotamers, maybe optimization issues
{
    if (_map == false)
    {
        for (int x = 0; x <= 34; x++)
        {
            submitJobAndRetrieve(x, Map);
        }
        _bouquet->storeRotamers(_allRotamer.resBouquet);

        if (_referential == false) // Rotating the helices to be aligned with the main referential
        {
            glm::mat3x3 R {_normal,_x,_y};


            _normal = glm::inverse(R) * _normal;
            _x = glm::inverse(R) * _x;
            _y = glm::inverse(R) * _y;

            _bouquet->updatePosition(R);
            std::cout << "position tt" << std::endl;
            _referential = true;
        }
        Result *r = new Result;
        _bouquet->storeRotRes();


        r->aps = _bouquet->extractForGUI(_bouquet->store);
        r->transplantPositions();
        r->destroy();
        _map = true;
    }
}

glm::vec3 RotamerModifier::axisForChain(points p, std::string chainName)
/* Plan :
 * - get two atoms at the start and end of each structures (considered to be alpha helices from start to end)
 * - draw an axis from them
 * - conpute them for both of the present helix
 */
{
    glm::vec3 firstPosition {};
    glm::vec3 secondPosition {};
    bool firstFound = false;
    for (auto atom : _group->atomVector())
    {
        if (atom->chain() == chainName && atom->isReporterAtom() && !firstFound)
        {
            firstPosition = atom->initialPosition();
            firstFound = true;
        }
        if (atom->chain() == chainName && atom->isReporterAtom() && firstFound)
        {
            secondPosition = atom->initialPosition();
        }
    }
    std::cout << "First atom position : " << firstPosition
    << " and second atom position : " << secondPosition << std::endl;
    _axis1 = secondPosition - firstPosition;
    glm::vec3 endLine = firstPosition +_axis1*glm::vec3(1.1);
    std::cout << "axisForChain :" << std::endl << "\tVector _axis1 :  " << _axis1 << std::endl;

    if (p == Start)
        return firstPosition-_axis1*glm::vec3(0.3);
    if (p == End)
        return endLine;
}
// bool RotamerModifier::isIntersection(CollisionBox &movingBox, CollisionBox &staticBox, glm::vec3 translation)
// {
//     return (
//     movingBox.xMin + translation.x <= staticBox.xMax &&
//     movingBox.xMax + translation.x >= staticBox.xMin &&
//     movingBox.yMin + translation.y <= staticBox.yMax &&
//     movingBox.yMax + translation.y >= staticBox.yMin &&
//     movingBox.zMin + translation.z <= staticBox.zMax &&
//     movingBox.zMax + translation.z >= staticBox.zMin
//     );
// }
// std::vector<glm::vec3> RotamerModifier::intersectionBox()
// {
//     std::vector<glm::vec3> intersection {};
//
//     if (isIntersection(_allRotamer.primaryBoxes["A"], _allRotamer.primaryBoxes["B"], (_xTrans+_yTrans))==false)
//     {
//         return intersection;
//     }
//     CollisionBox &movingBox = _allRotamer.primaryBoxes["A"];
//     CollisionBox &staticBox = _allRotamer.primaryBoxes["B"];
//     glm::vec3 translation {_xTrans + _yTrans};
//     glm::vec3 pMin {std::max(movingBox.xMin+ translation.x, staticBox.xMin),std::max(movingBox.yMin+ translation.y, staticBox.yMin),std::max(movingBox.zMin+ translation.z, staticBox.zMin)};
//     glm::vec3 pMax {std::min(movingBox.xMax+ translation.x, staticBox.xMax),std::min(movingBox.yMax+ translation.y, staticBox.yMax),std::min(movingBox.zMax+ translation.z, staticBox.zMax)};
//
//     intersection.emplace_back(pMin.x,pMin.y, pMin.z);
//     intersection.emplace_back(pMin.x,pMin.y, pMax.z);
//     intersection.emplace_back(pMin.x,pMax.y, pMax.z);
//     intersection.emplace_back(pMin.x,pMax.y, pMin.z);
//     intersection.emplace_back(pMin.x,pMin.y, pMin.z);
//     intersection.emplace_back(pMax.x,pMin.y, pMin.z);
//     intersection.emplace_back(pMax.x,pMin.y, pMax.z);
//     intersection.emplace_back(pMin.x,pMin.y, pMax.z);
//     intersection.emplace_back(pMax.x,pMin.y, pMax.z);
//     intersection.emplace_back(pMax.x,pMax.y, pMax.z);
//     intersection.emplace_back(pMin.x,pMax.y, pMax.z);
//     intersection.emplace_back(pMin.x,pMax.y, pMin.z);
//     intersection.emplace_back(pMax.x,pMax.y, pMin.z);
//     intersection.emplace_back(pMax.x,pMax.y, pMax.z);
//     intersection.emplace_back(pMax.x,pMax.y, pMin.z);
//     intersection.emplace_back(pMax.x,pMin.y, pMin.z);
//     return intersection;
// }
void RotamerModifier::makePlan() //NAME
/* Plan:
 * - take the reference helix (the one that will stay static)
 * - generate a plan perpendicular to this helix' axis
 * - create two vectors that will enable movement of the moving helix
 */
{
    glm::vec3 p1 = axisForChain(Start, "A");
    glm::vec3 p2 = axisForChain(End, "A");
    _normal = glm::normalize(p2 - p1); //_normal == axis of the A chain
    _x = glm::normalize(cross(_normal, vec3(1.0,0.0,0)));
    _y = glm:: normalize(cross(_normal, _x));
}
std::vector<glm::vec3> RotamerModifier::drawAxis()
{
    glm::vec3 startingPoint = {0,0,0};
    std::vector<glm::vec3> axis;
    axis.emplace_back(startingPoint);
    axis.emplace_back(_normal);
    axis.emplace_back(startingPoint);
    axis.emplace_back(_x);
    axis.emplace_back(startingPoint);
    axis.emplace_back(_y);
    return axis;
}


// void RotamerModifier::primaryCollisionBoxes()
// {
//     _allRotamer.primaryBoxes.clear();
//     for (auto &atom : _atomPosMap)
//     {
//         std::string chainName = atom.first->chain();
//         for (auto &pos : atom.second.samples)
//         {
//             if (!_allRotamer.primaryBoxes[chainName].xMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].xMin = pos.x;
//                 _allRotamer.primaryBoxes[chainName].xMax = pos.x;
//             }
//             else if (pos.x < _allRotamer.primaryBoxes[chainName].xMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].xMin = pos.x;
//             }
//             else if (pos.x > _allRotamer.primaryBoxes[chainName].xMax)
//             {
//                 _allRotamer.primaryBoxes[chainName].xMax = pos.x;
//             }
//             if (!_allRotamer.primaryBoxes[chainName].yMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].yMin = pos.y;
//                 _allRotamer.primaryBoxes[chainName].yMax = pos.y;
//             }
//             else if (pos.y < _allRotamer.primaryBoxes[chainName].yMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].yMin = pos.y;
//             }
//             else if (pos.y > _allRotamer.primaryBoxes[chainName].yMax)
//             {
//                 _allRotamer.primaryBoxes[chainName].yMax = pos.y;
//             }
//             if (!_allRotamer.primaryBoxes[chainName].zMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].zMin = pos.z;
//                 _allRotamer.primaryBoxes[chainName].zMax = pos.z;
//             }
//
//             if (pos.z < _allRotamer.primaryBoxes[chainName].zMin)
//             {
//                 _allRotamer.primaryBoxes[chainName].zMin = pos.z;
//             }
//             else if (pos.z > _allRotamer.primaryBoxes[chainName].zMax)
//             {
//                 _allRotamer.primaryBoxes[chainName].zMax = pos.z;
//             }
//         }
//     }
//     for (auto &box : _allRotamer.primaryBoxes)
//     {
//         _allRotamer.primaryBoxes[box.first].vertices.clear();
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMin, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMin, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMax, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMax, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMin, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMin, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMin, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMin, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMin, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMax, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMax, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMin,box.second.yMax, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMax, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMax, box.second.zMax);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMax, box.second.zMin);
//         _allRotamer.primaryBoxes[box.first].vertices.emplace_back(box.second.xMax,box.second.yMin, box.second.zMin);
//     }
//     std::cout << "boxes done" << std::endl;
// }
//
// void RotamerModifier::secondaryCollisionBoxes(std::map<ResidueId,CollisionBox> &secondaryBoxes, std::string chain, bool individual)
// {
//     secondaryBoxes.clear();
//
//     for (auto &atomPair : _atomPosMap)
//     {
//         std::string resName = atomPair.first->chain() + std::to_string(atomPair.first->residueNumber());
//         std::string resName1 {resName};
//
//         int counter = 0;
//         for (auto &pos : atomPair.second.samples)
//         {
//             if (secondaryBoxes[resName].name == "")
//                 secondaryBoxes[resName].name = resName;
//             if (individual)
//                 resName = resName1 + std::to_string(counter);
//             if (!secondaryBoxes[resName].xMin)
//             {
//                 secondaryBoxes[resName].xMin = pos.x;
//                 secondaryBoxes[resName].xMax = pos.x;
//             }
//             else if (pos.x < secondaryBoxes[resName].xMin)
//             {
//                 secondaryBoxes[resName].xMin = pos.x;
//             }
//             else if (pos.x > secondaryBoxes[resName].xMax)
//             {
//                 secondaryBoxes[resName].xMax = pos.x;
//             }
//             if (!secondaryBoxes[resName].yMin)
//             {
//                 secondaryBoxes[resName].yMin = pos.y;
//                 secondaryBoxes[resName].yMax = pos.y;
//             }
//             else if (pos.y < secondaryBoxes[resName].yMin)
//             {
//                 secondaryBoxes[resName].yMin = pos.y;
//             }
//             else if (pos.y > secondaryBoxes[resName].yMax)
//             {
//                 secondaryBoxes[resName].yMax = pos.y;
//             }
//             if (!secondaryBoxes[resName].zMin)
//             {
//                 secondaryBoxes[resName].zMin = pos.z;
//                 secondaryBoxes[resName].zMax = pos.z;
//             }
//
//             if (pos.z < secondaryBoxes[resName].zMin)
//             {
//                 secondaryBoxes[resName].zMin = pos.z;
//             }
//             else if (pos.z > secondaryBoxes[resName].zMax)
//             {
//                 secondaryBoxes[resName].zMax = pos.z;
//             }
//             ++counter;
//         }
//     }
//     for (auto &box : secondaryBoxes)
//     {
//         secondaryBoxes[box.first].vertices.clear();
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMin,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMax);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMax, secondaryBoxes[box.first].zMin);
//         secondaryBoxes[box.first].vertices.emplace_back(secondaryBoxes[box.first].xMax,secondaryBoxes[box.first].yMin, secondaryBoxes[box.first].zMin);
//     }
//     std::cout << "boxes done" << std::endl;
// }
// void RotamerModifier::IntersectionList(std::map<ResidueId, CollisionBox> &collisionBoxList)
// {
//     std::vector<glm::vec3> collision = intersectionBox();
//     CollisionBox instantCollision {};
//     _allRotamer.collisions.clear();
//     for (auto vector : collision)
//     {
//         if (!instantCollision.xMin)
//         {
//             instantCollision.xMin = vector.x;
//             instantCollision.xMax = vector.x;
//         }
//         else if (vector.x <= instantCollision.xMin)
//             instantCollision.xMin = vector.x;
//         else if (vector.x >= instantCollision.xMax)
//             instantCollision.xMax = vector.x;
//
//         if (!instantCollision.yMin)
//         {
//             instantCollision.yMin = vector.y;
//             instantCollision.yMax = vector.y;
//         }
//         else if (vector.y <= instantCollision.yMin)
//             instantCollision.yMin = vector.y;
//         else if (vector.y >= instantCollision.yMax)
//             instantCollision.yMax = vector.y;
//
//         if (!instantCollision.zMin)
//         {
//             instantCollision.zMin = vector.z;
//             instantCollision.zMax = vector.z;
//         }
//         else if (vector.z <= instantCollision.zMin)
//             instantCollision.zMin = vector.z;
//         else if (vector.z >= instantCollision.zMax)
//             instantCollision.zMax = vector.z;
//     }
//     for (auto rotamerBouquetPair : collisionBoxList)
//     {
//     if (isIntersection(rotamerBouquetPair.second, instantCollision,(rotamerBouquetPair.first.insert[0] == 'A' ? _xTrans+ _yTrans :  glm::vec3 {0,0,0} )))
//         {
//             collisionBoxList[rotamerBouquetPair.first].collision = true;
//             _allRotamer.collisions[(rotamerBouquetPair.first.insert[0] == 'A' ? "A" : "B")].emplace_back(rotamerBouquetPair.second);
//         //std::cout << "Residue " << rotamerBouquetPair.first << "\tis colliding"<< std::endl;
//         }
//     else
//         collisionBoxList[rotamerBouquetPair.first].collision = false;
//     }
// }
//
// void RotamerModifier::residueCollisions()
// {
//     for (auto collisionA : _allRotamer.collisions["A"])
//     {
//         for (auto collisionB : _allRotamer.collisions["B"])
//             if (isIntersection(collisionA,collisionB, _xTrans+ _yTrans))
//             {
//                 std::cout << "Collision between :\t" << collisionA.name << "\tand\t" << collisionB.name << std::endl;
//             }
//     }
// }

/*Monday's work Implementation of dynamic labelling of interacting secondary boxes, working towards a working third boxes collision test.
 *
 * Should I not stay on AABB and work with boxes adapted to each amino acids?
 * 
 */